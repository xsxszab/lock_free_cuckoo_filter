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

bool LockFreeCuckooFilter::find(const std::string& key, int tid) {
    return false;
}

bool LockFreeCuckooFilter::remove(const std::string& key, int tid) {
    return false;
}

int LockFreeCuckooFilter::size() const { return table_size; }
