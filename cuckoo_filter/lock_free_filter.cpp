#include <lock_free_filter.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>

LockFreeCuckooFilter::LockFreeCuckooFilter(int capacity, int _thread_count) {
    table_size = capacity;
    if (table_size <= 0) {
        std::cout << "error: invalid capacity" << std::endl;
        abort();
    }
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
    if (table_size <= 0) {
        std::cout << "error: invalid capacity" << std::endl;
        abort();
    }
    hash_table.resize(table_size);
    verbose = _verbose;

    hazard_ptrs.resize(_thread_count);
    for (int i = 0; i < _thread_count; i++) {
        hazard_ptrs[i].fill(nullptr);
    }
    retired_ptrs.resize(_thread_count);
    if (verbose) {
        std::cout << "Cuckoo Filter started" << std::endl;
    }
}

LockFreeCuckooFilter::~LockFreeCuckooFilter() {
    // delete all items stored in the hash table
    for (int i = 0; i < table_size; i++) {
        for (int j = 0; j < NUM_ITEMS_PER_ENTRY; j++) {
            HashEntry* ptr = (HashEntry*)get_real_pointer(hash_table[i][j]);
            if (ptr != nullptr) {
                delete ptr;
            }
        }
    }
    if (verbose) {
        std::cout << "Cuckoo Filter terminated" << std::endl;
    }
}

bool LockFreeCuckooFilter::insert(const std::string& key, const int tid) {
    clear_hazard(tid);
    if (verbose) {
        std::cout << "tid " << tid << ": try to insert key "
                  << key.substr(0, 16) << std::endl;
    }
    HashEntry* new_entry = new HashEntry();
    std::string fingerprint = md5_fingerprint(key);
    new_entry->str = fingerprint;
    int hash1 = jenkins_hash(key) % table_size;
    int hash2 = get_next_hash_index(hash1, fingerprint);

    table_pointer ptr1;
    while (true) {
        // iterate through all NUM_ITEMS_PER_ENTRY * 2 slots, try to find an
        // empty one
        bool found_empty = false;
        for (int i = 0; i < NUM_ITEMS_PER_ENTRY; i++) {
            ptr1 = hash_table[hash1][i];
            // found an empty slot
            if (get_real_pointer(ptr1) == 0) {
                found_empty = true;
                if (__sync_bool_compare_and_swap(
                        &hash_table[hash1][i], ptr1,
                        create_pointer(get_counter(ptr1), (uint64_t)new_entry,
                                       false))) {
                    if (verbose) {
                        std::cout << "tid " << tid
                                  << ": inserted key in bucket " << hash1
                                  << ", slot " << i << std::endl;
                    }
                    return true;
                }
            }

            ptr1 = hash_table[hash2][i];
            // found an empty slot
            if (get_real_pointer(ptr1) == 0) {
                found_empty = true;
                if (__sync_bool_compare_and_swap(
                        &hash_table[hash2][i], ptr1,
                        create_pointer(get_counter(ptr1), (uint64_t)new_entry,
                                       false))) {
                    if (verbose) {
                        std::cout << "tid " << tid
                                  << ": inserted key in bucket " << hash2
                                  << ", slot " << i << std::endl;
                    }
                    return true;
                }
            }
        }

        if (found_empty) {
            continue;
        }

        // cannot find empty slot, try to kick out one item
        unsigned seed =
            std::chrono::system_clock::now().time_since_epoch().count();

        std::array<int, NUM_ITEMS_PER_ENTRY> try_kick_order;
        for (int i = 0; i < NUM_ITEMS_PER_ENTRY; i++) {
            try_kick_order[i] = i;
        }

        std::shuffle(try_kick_order.begin(), try_kick_order.end(),
                     std::default_random_engine(seed));

        bool relocate_success = false;

        for (int i = 0; i < NUM_ITEMS_PER_ENTRY; i++) {
            int kick_slot = try_kick_order[i];
            if (relocate(hash1, kick_slot, tid)) {
                relocate_success = true;
                break;
            }
        }

        if (!relocate_success) {
            if (verbose) {
                std::cout << "tid " << tid
                          << ": warning: table full, inseration failed"
                          << std::endl;
            }
            return false;
        }
    }

    // shut up, gcc
    return false;
}

int LockFreeCuckooFilter::find(const std::string& key, const int tid) {
    table_pointer place_holder;
    return find(key, place_holder, tid);
}

int LockFreeCuckooFilter::find(const std::string& key, table_pointer& pointer,
                               const int tid) {
    clear_hazard(tid);
    if (verbose) {
        std::cout << "tid " << tid << ": try to find key " << key.substr(0, 16)
                  << std::endl;
    }
    std::string fingerprint = md5_fingerprint(key);
    uint32_t hash1 = jenkins_hash(key) % table_size;
    uint32_t hash2 = get_next_hash_index(hash1, fingerprint);

    // iterate for every slot in an table entry
    for (int slot = 0; slot < NUM_ITEMS_PER_ENTRY; slot++) {
        while (true) {
            // first round search start
            table_pointer ptr1 = hash_table[hash1][slot];
            mark_hazard((HashEntry*)get_real_pointer(ptr1), 0, tid);
            int ts1 = get_counter(ptr1);
            if (get_real_pointer(ptr1) != 0) {  // first location not null
                if (get_marked(ptr1)) {         // help relocate this item
                    help_relocate(hash1, slot, false, tid);
                    continue;
                } else {
                    HashEntry* real_ptr = (HashEntry*)get_real_pointer(ptr1);
                    if (fingerprint == real_ptr->str) {  // item found
                        pointer = ptr1;
                        if (verbose) {
                            std::cout << "tid " << tid
                                      << ": found key in bucket hash1, slot "
                                      << slot << std::endl;
                        }
                        return slot;
                    }
                }
            }

            table_pointer ptr2 = hash_table[hash2][slot];
            int ts2 = get_counter(ptr2);
            mark_hazard((HashEntry*)get_real_pointer(ptr2), 1, tid);
            if (get_real_pointer(ptr2) != 0) {  // second location not null
                if (get_marked(ptr2)) {         // help relocate this item
                    help_relocate(hash2, slot, false, tid);
                    continue;
                } else {
                    HashEntry* real_ptr = (HashEntry*)get_real_pointer(ptr2);
                    if (fingerprint == real_ptr->str) {  // item found
                        pointer = ptr2;
                        if (verbose) {
                            std::cout << "tid " << tid
                                      << ": found key in bucket hash2, slot "
                                      << slot << std::endl;
                        }
                        return slot + NUM_ITEMS_PER_ENTRY;
                    }
                }
            }
            // first round search end, second round search start
            int ts1x = get_counter(hash_table[hash1][slot]);
            int ts2x = get_counter(hash_table[hash2][slot]);
            if (check_counter(ts1, ts2, ts1x, ts2x)) {
                // possible false missing, try again
                if (verbose) {
                    std::cout << "tid " << tid
                              << ": try find again after cheecking counter"
                              << std::endl;
                }
                continue;
            } else {
                // key not found in current slot
                break;
            }
            // second round search end
        }
    }

    if (verbose) {
        std::cout << "tid " << tid << ": key not found" << std::endl;
    }
    return NUM_ITEMS_PER_ENTRY * 2;
}

bool LockFreeCuckooFilter::remove(const std::string& key, const int tid) {
    clear_hazard(tid);
    std::string fingerprint = md5_fingerprint(key);
    uint32_t hash1 = jenkins_hash(key) % table_size;
    uint32_t hash2 = get_next_hash_index(hash1, fingerprint);
    table_pointer ptr1;
    while (true) {
        int location = find(key, ptr1, tid);
        // not found
        if (location == NUM_ITEMS_PER_ENTRY * 2) {
            if (verbose) {
                std::cout << "tid " << tid
                          << ": warning: try to delete nonexistent key"
                          << std::endl;
            }
            return false;
        }
        int hash_val = location < NUM_ITEMS_PER_ENTRY ? hash1 : hash2;
        if (__sync_bool_compare_and_swap(
                &hash_table[hash_val][location % NUM_ITEMS_PER_ENTRY], ptr1,
                create_pointer(get_counter(ptr1), (uint64_t) nullptr, false))) {
            if (verbose) {
                std::cout << "tid " << tid
                          << ": deleted key at table index: " << hash_val
                          << " , slot: " << location % NUM_ITEMS_PER_ENTRY
                          << std::endl;
            }
            retire_key(ptr1, tid);
            return true;
        }
    }

    // shut up, gcc
    return false;
}

int LockFreeCuckooFilter::size() const { return table_size; }

void LockFreeCuckooFilter::change_verbose(const bool _verbose) {
    verbose = _verbose;
}

void LockFreeCuckooFilter::reset(const int capacity, const int _thread_count) {
    for (int i = 0; i < table_size; i++) {
        for (int j = 0; j < NUM_ITEMS_PER_ENTRY; j++) {
            HashEntry* ptr = (HashEntry*)get_real_pointer(hash_table[i][j]);
            if (ptr != nullptr) {
                delete ptr;
            }
        }
    }

    hash_table.clear();
    hazard_ptrs.clear();
    retired_ptrs.clear();

    table_size = capacity;
    thread_count = _thread_count;
    hash_table.resize(table_size);
    hazard_ptrs.resize(thread_count);
    for (int i = 0; i < thread_count; i++) {
        hazard_ptrs[i].fill(nullptr);
    }
    retired_ptrs.resize(thread_count);
}

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

int LockFreeCuckooFilter::get_next_hash_index(const int curr_idx,
                                              const std::string fingerprint) {
    return (curr_idx ^ (jenkins_hash(fingerprint) % table_size)) % table_size;
}

void LockFreeCuckooFilter::mark_hazard(HashEntry* real_pointer, int index,
                                       int tid) {
    hazard_ptrs[tid][index] = real_pointer;
}

void LockFreeCuckooFilter::clear_hazard(int tid) {
    mark_hazard(nullptr, 0, tid);
    mark_hazard(nullptr, 1, tid);
}

// int LockFreeCuckooFilter::mark_hazard(table_pointer pointer, int tid) {
//     for (int i = 0; i < (int)hazard_ptrs[tid].size(); i++) {
//         if (hazard_ptrs[tid][i] == nullptr) {
//             hazard_ptrs[tid][i] = (HashEntry*)get_real_pointer(pointer);
//             return i;
//         }
//     }
//     // should not happen
//     return hazard_ptrs[tid].size();
// }

// void LockFreeCuckooFilter::update_hazard(table_pointer pointer, int index,
//                                          int tid) {
//     hazard_ptrs[tid][index] = (HashEntry*)get_real_pointer(pointer);
// }

// void LockFreeCuckooFilter::unmark_hazard(int index, int tid) {
//     hazard_ptrs[tid][index] = nullptr;
// }

void LockFreeCuckooFilter::retire_key(table_pointer pointer, const int tid) {
    retired_ptrs[tid].push_back((HashEntry*)get_real_pointer(pointer));
}

void LockFreeCuckooFilter::help_relocate(int table_idx, int slot_idx,
                                         bool initiator, int tid) {
    clear_hazard(tid);
    if (verbose) {
        std::cout << "tid " << tid << ": help relocate item in bucket "
                  << table_idx << ", slot " << slot_idx << std::endl;
    }
    while (1) {
        table_pointer ptr1 = hash_table[table_idx][slot_idx];
        HashEntry* real_ptr1 = (HashEntry*)get_real_pointer(ptr1);
        mark_hazard(real_ptr1, 0, tid);
        while (initiator && !get_marked(ptr1)) {
            if (real_ptr1 == nullptr) {
                return;
            }

            // mark ptr1 as being relocated
            __sync_bool_compare_and_swap(&hash_table[table_idx][slot_idx], ptr1,
                                         get_new_mark(ptr1, true));
            ptr1 = hash_table[table_idx][slot_idx];
            mark_hazard((HashEntry*)get_real_pointer(ptr1), 0, tid);
            real_ptr1 = (HashEntry*)get_real_pointer(ptr1);
        }

        if (!get_marked(ptr1)) {
            return;
        }

        int new_hash = get_next_hash_index(table_idx, real_ptr1->str);
        table_pointer ptr2 = hash_table[new_hash][slot_idx];
        HashEntry* real_ptr2 = (HashEntry*)get_real_pointer(ptr2);
        mark_hazard(real_ptr2, 1, tid);

        uint16_t counter1 = get_counter(ptr1);
        uint16_t counter2 = get_counter(ptr2);

        if (real_ptr2 == nullptr) {  // destination is empty
            int n_count = counter1 > counter2 ? counter1 + 1 : counter2 + 1;

            // make sure ptr1 hasn't been changed
            if (ptr1 != hash_table[table_idx][slot_idx]) {
                continue;
            }

            // move pointer to destination slot
            if (__sync_bool_compare_and_swap(
                    &hash_table[new_hash][slot_idx], ptr2,
                    create_pointer(n_count, (uint64_t)real_ptr1, false))) {
                // clear source slot
                __sync_bool_compare_and_swap(
                    &hash_table[table_idx][slot_idx], ptr1,
                    create_pointer(counter1 + 1, (uint64_t) nullptr, false));
                return;
            }
        }
        // relocation already completed by another thread
        if (real_ptr1 == real_ptr2) {
            __sync_bool_compare_and_swap(
                &hash_table[new_hash][slot_idx], ptr1,
                create_pointer(counter1 + 1, (uint64_t) nullptr, false));
            return;
        }

        // relocation failed, because the destination slot has already been
        // occupied by another key
        __sync_bool_compare_and_swap(
            &hash_table[new_hash][slot_idx], ptr1,
            create_pointer(counter1 + 1, (uint64_t)real_ptr1, true));
        return;
    }

    // shut up, gcc
    return;
}

void LockFreeCuckooFilter::free_hazard_pointers(int tid) {
    if (verbose) {
        std::cout << "tid " << tid << ": try to free retired items"
                  << std::endl;
    }
    std::unordered_set<HashEntry*> hazard_record;

    // record all hazard pointers using an unordered set
    for (int i = 0; i < thread_count; i++) {
        for (int j = 0; j < MAX_HAZARD_POINTER_COUNT; j++) {
            HashEntry* ptr = hazard_ptrs[i][j];
            if (ptr != nullptr) {
                hazard_record.insert(ptr);
            }
        }
    }

    int old_count = retired_ptrs[tid].size();
    int new_count = 0;

    // since new_count will always be smaller than or equal to old_count, the
    // following code won't cause out-of-bound error
    for (int i = 0; i < old_count; i++) {
        // retired pointer still in use, cannot free it
        if (hazard_record.count(retired_ptrs[tid][i])) {
            retired_ptrs[tid][new_count++] = retired_ptrs[tid][i];
        } else {  // say goodbye to the retired pointer
            delete retired_ptrs[tid][i];
        }
    }

    if (verbose) {
        std::cout << "tid " << tid << ": successfully cleared " << old_count
                  << " retired items" << std::endl;
    }
    retired_ptrs[tid].resize(new_count);
}

bool LockFreeCuckooFilter::relocate(int table_idx, int slot_idx, int tid) {
    if (verbose) {
        std::cout << "tid " << tid << ": relocate item in bucket " << table_idx
                  << ", slot " << slot_idx << std::endl;
    }

    int route[NUM_MAX_KICKS];
    int start_level = 0;

path_discovery:
    bool found = false;
    int depth = start_level;
    for (int i = 0; i < NUM_MAX_KICKS; i++) {
        route[i] = 0;
    }
    if (verbose) {
        std::cout << "tid " << tid << ": start path discovery" << std::endl;
    }
    do {
        table_pointer ptr1 = hash_table[table_idx][slot_idx];
        mark_hazard((HashEntry*)get_real_pointer(ptr1), 0, tid);
        // help relocate destionation slot until it is unmarked
        while (get_marked(ptr1)) {
            help_relocate(table_idx, slot_idx, false, tid);
            ptr1 = hash_table[table_idx][slot_idx];
        }

        // destionation slot already occupied
        if ((HashEntry*)get_real_pointer(ptr1) != nullptr) {
            route[depth] = table_idx;
            HashEntry* real_ptr1 = (HashEntry*)get_real_pointer(ptr1);
            // calculate replacement location
            table_idx = get_next_hash_index(table_idx, real_ptr1->str);
        } else {
            found = true;
        }

        // loop until a valid replacement path is found or maximum depth reached
    } while (!found && ++depth < NUM_MAX_KICKS);

    depth--;

    if (found) {
        // traverse the path in reverse order
        for (int i = depth; i >= 0; i--) {
            int source_idx = route[depth];
            table_pointer source_ptr = hash_table[source_idx][slot_idx];
            if (get_marked(source_ptr)) {
                help_relocate(source_idx, slot_idx, false, tid);
                source_ptr = hash_table[source_idx][slot_idx];
            }
            HashEntry* real_ptr1 = (HashEntry*)get_real_pointer(source_ptr);
            if (real_ptr1 == nullptr) {
                continue;
            }
            int dest_idx = get_next_hash_index(source_idx, real_ptr1->str);
            table_pointer dest_ptr = hash_table[dest_idx][slot_idx];
            mark_hazard((HashEntry*)get_real_pointer(dest_ptr), 1, tid);
            if ((HashEntry*)get_real_pointer(dest_ptr) != nullptr) {
                start_level = i + 1;
                goto path_discovery;
            }
            help_relocate(source_idx, slot_idx, true, tid);
        }
    }

    return found;
}
