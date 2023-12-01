#include <lock_free_filter.h>

LockFreeCuckooFilter::LockFreeCuckooFilter(int capacity, int _thread_count) {
    table_size = capacity;
    hash_table.resize(table_size);
    verbose = false;
}

LockFreeCuckooFilter::LockFreeCuckooFilter(int capacity, int _thread_count,
                                           bool _verbose) {
    table_size = capacity;
    hash_table.resize(table_size);
    verbose = _verbose;
}

LockFreeCuckooFilter::~LockFreeCuckooFilter() {}

bool LockFreeCuckooFilter::insert(const std::string& key, const int tid) {
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

bool LockFreeCuckooFilter::check_counter(const int ts1, const int ts2,
                                         const int ts1x, const int ts2x) {
    bool cond1 = ts1x - ts1 >= 2;  // slot 1 has been updated at least 2 times
    bool cond2 = ts2x - ts2 >= 2;  // slot 2 has been updated at least 2 times
    bool cond3 = ts2x - ts1 >= 3;  // slot 2's new counter value minus slot 1's
                                   // initial value should be at least 3
    // if all conditions are met, it is possible that the query misses an item
    return cond1 && cond2 && cond3;
}

void LockFreeCuckooFilter::retire_key(table_pointer pointer, const int tid) {}
