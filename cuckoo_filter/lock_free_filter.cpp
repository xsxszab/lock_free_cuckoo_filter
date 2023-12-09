#include <lock_free_filter.h>

LockFreeCuckooFilter::LockFreeCuckooFilter(int capacity, int _thread_count) {
    table_size = capacity;
    hash_table.resize(table_size);
    verbose = false;

    hazard_ptrs.resize(_thread_count);
    for (int i = 0; i < _thread_count; i++) {
        hazard_ptrs[i].fill(nullptr);
    }
    retired_ptrs.resize(_thread_count);
}

LockFreeCuckooFilter::LockFreeCuckooFilter(int capacity, int _thread_count,
                                           bool _verbose) {
    table_size = capacity;
    hash_table.resize(table_size);
    verbose = _verbose;

    hazard_ptrs.resize(_thread_count);
    for (int i = 0; i < _thread_count; i++) {
        hazard_ptrs[i].fill(nullptr);
    }
    retired_ptrs.resize(_thread_count);
}

LockFreeCuckooFilter::~LockFreeCuckooFilter() {}

bool LockFreeCuckooFilter::insert(const std::string& key, const int tid) {
    HashEntry* new_entry = new HashEntry();
    new_entry->str = key;
    std::string fingerprint = md5_fingerprint(key);
    uint32_t hash1 = jenkins_hash(key) % table_size;
    uint32_t hash2 = hash1 ^ (jenkins_hash(fingerprint) % table_size);

    table_pointer ptr1;
    while (true) {
        int location = find(key, ptr1, tid);
        // TODO
    }
    return false;
}

int LockFreeCuckooFilter::find(const std::string& key, const int tid) {
    // TODO: a modified version of find that does not return found pointer
    return 0;
}

int LockFreeCuckooFilter::find(const std::string& key, table_pointer& pointer,
                               const int tid) {
    // TODO: hazard pointer related stuff
    std::string fingerprint = md5_fingerprint(key);
    uint32_t hash1 = jenkins_hash(key) % table_size;
    uint32_t hash2 = hash1 ^ (jenkins_hash(fingerprint) % table_size);
    // iterate for every slot in an table entry
    for (int slot = 0; slot < NUM_ITEMS_PER_ENTRY; slot++) {
        while (true) {
            // first round search start
            table_pointer ptr1 = hash_table[hash1][slot];
            mark_hazard(ptr1, tid);
            int ts1 = get_counter(ptr1);
            if (get_real_pointer(ptr1) != 0) {  // first location not null
                if (get_marked(ptr1)) {         // help relocate this item
                                                // TODO: help relocate
                    continue;
                } else if (fingerprint == *(std::string*)get_real_pointer(
                                              ptr1)) {  // item found, return
                    pointer = ptr1;
                    return slot;
                }
            }

            table_pointer ptr2 = hash_table[hash2][slot];
            int ts2 = get_counter(ptr2);
            if (get_real_pointer(ptr2) != 0) {  // second location not null
                if (get_marked(ptr2)) {         // help relocate this item
                    // TODO: help relocate
                    continue;
                } else if (fingerprint ==
                           *(std::string*)get_real_pointer(ptr2)) {
                    pointer = ptr2;
                    return slot + NUM_ITEMS_PER_ENTRY;
                }
            }
            // first round search end, second round search start
            int ts1x = get_counter(hash_table[hash1][slot]);
            int ts2x = get_counter(hash_table[hash2][slot]);
            if (check_counter(ts1, ts2, ts1x, ts2x)) {
                // possible false missing, try again
                continue;
            }
            // second round search end
        }
    }
    // key not found
    return NUM_ITEMS_PER_ENTRY * 2;
}

bool LockFreeCuckooFilter::remove(const std::string& key, const int tid) {
    std::string fingerprint = md5_fingerprint(key);
    uint32_t hash1 = jenkins_hash(key) % table_size;
    uint32_t hash2 = hash1 ^ (jenkins_hash(fingerprint) % table_size);

    return false;
}

int LockFreeCuckooFilter::size() const { return table_size; }

// void LockFreeCuckooFilter::mark_hazard(table_pointer pointer, int tid) {
//     hazard_ptrs[tid].push_back((HashEntry*)get_real_pointer(pointer));
// }

bool LockFreeCuckooFilter::check_counter(const int ts1, const int ts2,
                                         const int ts1x, const int ts2x) {
    bool cond1 = ts1x - ts1 >= 2;  // slot 1 has been updated at least 2 times
    bool cond2 = ts2x - ts2 >= 2;  // slot 2 has been updated at least 2 times
    bool cond3 = ts2x - ts1 >= 3;  // slot 2's new counter value minus slot 1's
                                   // initial value should be at least 3
    // if all conditions are met, it is possible that the query misses an item
    return cond1 && cond2 && cond3;
}

int LockFreeCuckooFilter::mark_hazard(table_pointer pointer, int tid) {
    for (int i = 0; i < (int)hazard_ptrs[tid].size(); i++) {
        if (hazard_ptrs[tid][i] == nullptr) {
            hazard_ptrs[tid][i] = (HashEntry*)get_real_pointer(pointer);
            return i;
        }
    }
    // should not happen
    return hazard_ptrs[tid].size();
}

void LockFreeCuckooFilter::update_hazard(table_pointer pointer, int index,
                                         int tid) {
    hazard_ptrs[tid][index] = (HashEntry*)get_real_pointer(pointer);
}

void LockFreeCuckooFilter::unmark_hazard(int index, int tid) {
    hazard_ptrs[tid][index] = nullptr;
}

void LockFreeCuckooFilter::retire_key(table_pointer pointer, const int tid) {
    retired_ptrs[tid].push_back((HashEntry*)get_real_pointer(pointer));
}

void LockFreeCuckooFilter::help_relocate(int table_idx, int slot_idx,
                                         bool initiator, int tid) {
    while (1) {
        table_pointer ptr1 = hash_table[table_idx][slot_idx];
        HashEntry* real_ptr1 = (HashEntry*)get_real_pointer(ptr1);
        int hazard_idx1 = mark_hazard(ptr1, tid);
        while (initiator && !get_marked(ptr1)) {
            if (real_ptr1 == nullptr) {
                unmark_hazard(hazard_idx1, tid);
                return;
            }
            __sync_bool_compare_and_swap(&hash_table[table_idx][slot_idx], ptr1,
                                         get_new_mark(ptr1, true));
            ptr1 = hash_table[table_idx][slot_idx];
            update_hazard(ptr1, hazard_idx1, tid);
            // entry has been modified by other threads, try again
            if (ptr1 != hash_table[table_idx][slot_idx]) {
                continue;
            }
            real_ptr1 = (HashEntry*)get_real_pointer(ptr1);
        }

        if (!get_marked(ptr1)) {
            unmark_hazard(hazard_idx1, tid);
            return;
        }

        int new_hash = table_idx ^ (jenkins_hash(real_ptr1->str) % table_size);
        table_pointer ptr2 = hash_table[new_hash][slot_idx];
        HashEntry* real_ptr2 = (HashEntry*)get_real_pointer(ptr2);
        int hazard_idx2 = mark_hazard(ptr2, tid);
        if (ptr2 != hash_table[new_hash][slot_idx]) {
            continue;
        }

        uint16_t counter1 = get_counter(ptr1);
        uint16_t counter2 = get_counter(ptr2);

        if (real_ptr2 == nullptr) {
            int n_count = counter1 > counter2 ? counter1 + 1 : counter2 + 1;
            if (ptr1 != hash_table[table_idx][slot_idx]) {
                continue;
            }

            if (__sync_bool_compare_and_swap(
                    &hash_table[new_hash][slot_idx], ptr2,
                    create_pointer(n_count, (uint64_t)real_ptr1, false))) {
                __sync_bool_compare_and_swap(
                    &hash_table[table_idx][slot_idx], ptr1,
                    create_pointer(counter1 + 1, (uint64_t) nullptr, false));

                unmark_hazard(hazard_idx1, tid);
                unmark_hazard(hazard_idx2, tid);
                return;
            }
        }
        // relocation already completed by another thread
        if (real_ptr1 == real_ptr2) {
            __sync_bool_compare_and_swap(
                &hash_table[new_hash][slot_idx], ptr1,
                create_pointer(counter1 + 1, (uint64_t) nullptr, false));
            unmark_hazard(hazard_idx1, tid);
            unmark_hazard(hazard_idx2, tid);
            return;
        }

        // relocation failed, because the destination slot has already been
        // occupied by another key
        __sync_bool_compare_and_swap(
            &hash_table[new_hash][slot_idx], ptr1,
            create_pointer(counter1 + 1, (uint64_t)real_ptr1, true));
        unmark_hazard(hazard_idx1, tid);
        unmark_hazard(hazard_idx2, tid);
        return;
    }
}

void LockFreeCuckooFilter::free_hazard_pointers(int tid) {
    std::unordered_set<HashEntry*> record;

    // record all hazard pointers using an unordered set
    for (int i = 0; i < thread_count; i++) {
        for (int j = 0; j < MAX_HAZARD_POINTER_COUNT; j++) {
            HashEntry* ptr = hazard_ptrs[i][j];
            if (ptr != nullptr) {
                record.insert(ptr);
            }
        }
    }

    int old_count = retired_ptrs[tid].size();
    int new_count = 0;

    // since new_count will always be smaller than or equal to old_count, this
    // won't cause out-of-bound error
    for (int i = 0; i < old_count; i++) {
        // retired pointer still in use, cannot free it
        if (record.count(retired_ptrs[tid][i])) {
            retired_ptrs[tid][new_count++] = retired_ptrs[tid][i];
        } else {  // say goodbye to the retired pointer
            delete retired_ptrs[tid][i];
        }
    }
    retired_ptrs[tid].resize(new_count);
}

bool LockFreeCuckooFilter::relocate(int table_idx, int slot_idx, int tid) {
    return false;
}
