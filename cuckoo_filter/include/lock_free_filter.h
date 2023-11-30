#ifndef LOCK_FREE_FILTER
#define LOCK_FREE_FILTER

#include <common.h>
#include <marcos.h>
#include <pointer_utils.h>

#include <array>
#include <string>
#include <vector>

// a lock-free cuckoo filter implementation
class LockFreeCuckooFilter {
   public:
    LockFreeCuckooFilter(int capacity, int _thread_count);
    LockFreeCuckooFilter(int capacity, int _thread_count, bool _verbose);
    ~LockFreeCuckooFilter();

    // self-explanatory
    DISABLE_COPY_AND_MOVE_CONSTRUCT(LockFreeCuckooFilter)

    // insert a key into the hash table. Return true if successfully inserted
    // new key, false is the insertion failed (hash table full). tid: caller's
    // thread id.
    bool insert(const std::string& key, int tid);
    // find is a given key is present in the hash table. Return true if present,
    // false if not. tid: caller's thread id.
    bool find(const std::string& key, int tid);

    // remove a key from the hash table. Return true if the key is deleted,
    // false if the key is not present in the table. Note that when using this
    // filter, it is the user's responsibility to ensure that any key must be in
    // the table before deletion, i.e., this function must always return true,
    // otherwise the filter's behavior is undefined. tid: caller's thread id
    bool remove(const std::string& key, int tid);

    // return the hash table's size (number of entries, not the table_size *
    // NUM_ITEMS_PER_ENTRY). tid: caller's thread id.
    int size(int tid) const;

   private:
    int table_size;
    int thread_count;
    std::vector<std::array<table_pointer, NUM_ITEMS_PER_ENTRY>> hash_table;
    bool verbose;
};

#endif
