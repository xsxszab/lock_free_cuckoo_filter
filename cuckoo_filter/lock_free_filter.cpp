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

bool LockFreeCuckooFilter::insert(const std::string& key, int tid) {
    return false;
}

int LockFreeCuckooFilter::find(const std::string& key, int tid) {
    return false;
}

bool LockFreeCuckooFilter::remove(const std::string& key, int tid) {
    return false;
}

int LockFreeCuckooFilter::size() const { return table_size; }

bool LockFreeCuckooFilter::check_counter(int ts1, int ts2, int ts1x, int ts2x) {
    bool cond1 = ts1x - ts1 >= 2;  // slot 1 has been updated at least 2 times
    bool cond2 = ts2x - ts2 >= 2;  // slot 2 has been updated at least 2 times
    bool cond3 = ts2x - ts1 >= 3;  // slot 2's new counter value minus slot 1's
                                   // initial value should be at least 3
    // if all conditions are met, it is possible that the query misses an item
    return cond1 && cond2 && cond3;
}
