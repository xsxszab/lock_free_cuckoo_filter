#ifndef LOCK_FREE_FILTER
#define LOCK_FREE_FILTER

/**
 * @file lock_free_filter.h
 * @brief this file contains the defintion of a lock-free cuckoo filter
 */

#include <common.h>
#include <marcos.h>
#include <pointer_utils.h>

#include <array>
#include <string>
#include <vector>

/**
 * @brief A lock-free cuckoo filter implementation. Note that since the cuckoo
 * filter is implemented based on the cuckoo hashing table, the terms 'cuckoo
 * filter' and 'cuckoo hash table' are used interchangeably.
 * */
class LockFreeCuckooFilter {
   public:
    /**
     * @brief construct a lock free cuckoo filter instance
     * @param[in] capacity size of the hash table
     * @param[in] _thread_count Number of threads that will access this filter
     * concurrently.
     */
    LockFreeCuckooFilter(int capacity, int _thread_count);

    /**
     * @brief Construct a lock free cuckoo filter instance.
     * @param[in] capacity Size of the hash table.
     * @param[in] _thread_count Number of threads that will access this filter
     * concurrently.
     * @param[in] _verbose Whether or not to print out debug information.
     */
    LockFreeCuckooFilter(int capacity, int _thread_count, bool _verbose);

    /**
     * @brief Destory the cuckoo filter.
     */
    ~LockFreeCuckooFilter();

    // self-explanatory
    DISABLE_COPY_AND_MOVE_CONSTRUCT(LockFreeCuckooFilter)

    /**
     * @brief Insert a key into the hash table.
     * @param[in] key the key to be inserted.
     * @param[in] tid the caller's thread id
     *
     * @return true if successfully inserted the key, false if failed (hash
     * table full).
     */
    bool insert(const std::string& key, int tid);

    /**
     * @brief Find if a given key is present in the filter.
     * @param[in]  key The key to search.
     * @param[in] tid the caller's thread id
     *
     * @return 0 if not found, 1 if key is in the primary entry, 2 if key is in
     * the secondary entry
     */
    int find(const std::string& key, int tid);

    /**
     * @brief Remove a key from the hash table.
     * @param[in] key The key to delete.
     * @param[in] tid The caller's thread id.
     *
     * @return true if successfully deleted the key, false if not. Note that it
     * is the user's responsibility to ensure that any key must be present
     * before deletion, i.e., this function must always return true, otherwise
     * the filter's behavior is undefined.
     */
    bool remove(const std::string& key, int tid);

    // return the hash table's size (number of entries, not the table_size *
    // NUM_ITEMS_PER_ENTRY). tid: caller's thread id.

    /**
     * @brief get the hash table's size.
     * @return return the hash table's size.
     */
    int size() const;

   private:
    /**
     * @brief The hash table's size (number of entries).
     */
    int table_size;

    /**
     * @brief Number of threads that will access this filter concurrently.
     */
    int thread_count;

    /**
     * @brief The internal hash table.
     */
    std::vector<std::array<table_pointer, NUM_ITEMS_PER_ENTRY>> hash_table;

    /**
     * @brief Verbose flag.
     */
    bool verbose;

    /**
     * @brief check counter value.
     * For detailed explanation please refer to
     * https://ieeexplore.ieee.org/document/6888938, page 4
     * @return true if a false miss might happened, false if not
     */
    bool check_counter(int ts1, int ts2, int ts1x, int ts2x);
};

#endif
