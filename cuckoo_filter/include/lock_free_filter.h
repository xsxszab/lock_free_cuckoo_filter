#ifndef LOCK_FREE_FILTER
#define LOCK_FREE_FILTER

/**
 * @file lock_free_filter.h
 * @brief this file contains the defintion of a lock-free cuckoo filter
 */

#include <common.h>
#include <hash_utils.h>
#include <marcos.h>
#include <pointer_utils.h>

#include <array>
#include <new>
#include <string>
#include <unordered_set>
#include <vector>

#define MAX_RETIRED_POINTER_COUNT 128
#define MAX_HAZARD_POINTER_COUNT 2

// const int hardware_destructive_interference_size = 128;

/**
 * @brief hash table entry
 */
typedef struct {
    std::string str;  // stores fingerprint
} HashEntry;

// typedef struct {
//     alignas(hardware_destructive_interference_size) int val;
// } AlignedInt;

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
    bool insert(const std::string& key, const int tid);

    /**
     * @brief Find if a given key is present in the filter.
     * @param[in]  key The key to search.
     * @param[in] tid the caller's thread id
     *
     * @return (TODO: return slot index, need better explanation)
     */
    int find(const std::string& key, const int tid);

    /**
     * @brief Find if a given key is present in the filter. If it is present,
     * return the corresponding table_pointer.
     * @param[in]  key The key to search.
     * @param[in, out] pointer if the key is found, this table_pointer points to
     * the corresponding table item.
     * @param[in] tid the caller's thread id
     * @return (TODO: return slot index, need better explanation)
     */
    int find(const std::string& key, table_pointer& pointer, const int tid);

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
    bool remove(const std::string& key, const int tid);

    /**
     * @brief Get the hash table's size.
     * @return return the hash table's size.
     */
    int size() const;

    /**
     * @brief Change the filter's verbose mode.
     * @param[in] _verbose new verbose mode
     */
    void change_verbose(const bool _verbose);

    /**
     * @brief Reset the filter with new capacity and thread count. Warning: this
     * method can only be called when no thread will access this filter anymore,
     * otherwise the filter's behavior is undefined.
     * @param[in] capacity hash table size
     * @param[in] _thread_count number of threads that will access this filter
     * concurrently
     */
    void reset(const int capacity, const int _thread_count);

   private:
    /**
     * @brief The hash table's size (number of entries). Since cuckoo filter
     * only store item's fingerprint, it is impossible to enlarge the table
     * through rehashing, therefore this value will remain constant.
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
     * @brief A per-thread record of hazard pointers.
     */
    std::vector<std::array<HashEntry*, MAX_HAZARD_POINTER_COUNT>> hazard_ptrs;

    // std::vector<AlignedInt> hazard_ptr_count;

    /**
     * @brief A per-thread record of retired pointers, denoting pointers that
     * are no longer used by the current thread but may be accessed by other
     * threads.
     */
    std::vector<std::vector<HashEntry*>> retired_ptrs;

    // std::vector<AlignedInt> retired_ptr_count;

    /**
     * @brief check counter value.
     * For detailed explanation please refer to
     * https://ieeexplore.ieee.org/document/6888938, page 4
     * @return true if a false miss might happened, false if not
     */
    bool check_counter(const int ts1, const int ts2, const int ts1x,
                       const int ts2x);

    /**
     * @brief Calculate a key's next hash table index based on the current index
     * and its fingerprint.
     * @param[in] curr_idx current index
     * @param[in] fingerprint the key's MD5 fingerprint
     */
    int get_next_hash_index(const int curr_idx, const std::string fingerprint);

    // int mark_hazard(table_pointer pointer, int tid);

    // void update_hazard(table_pointer pointer, int index, int tid);

    // void unmark_hazard(int index, int tid);

    /**
     * @brief mark a pointer as hazard to prevent it from being freed by other
     * threads
     * @param[in] real_pointer the pointer to mark
     * @param[in] index the index of hazard_ptrs
     * @param[in] tid caller's thread id
     */
    void mark_hazard(HashEntry* real_pointer, int index, int tid);

    /**
     * @brief clear current thread's hazard pointer list
     * @param[in] tid caller's thread id
     */
    void clear_hazard(int tid);

    /**
     * @brief mark a pointer as retired by putting it into retired_ptrs. Note
     * that only the real pointer part of the table pointer will be stored.
     * @param[in] pointer a table pointer
     * @param[in] tid caller's thread id
     */
    void retire_key(table_pointer pointer, const int tid);

    /**
     * @brief help relocate a marked item to its alternative location.
     * @param[in] table_idx the item's index in the hash table
     * @param[in] slot_idx which slot the item is in
     * @param[in] tid caller's thread id
     */
    void help_relocate(int table_idx, int slot_idx, bool initiator, int tid);

    /**
     * @brief free current thread's retired pointers that are not being used by
     * any thread, then remove them from retired_ptrs.
     * @param[in] tid caller's thread id
     */
    void free_hazard_pointers(int tid);

    /**
     * @brief relocate an item to make its original slot empty.
     * @param[in] table_idx the item's index in the hash table
     * @param[in] slot_idx which slot the item is in
     * @param[in] tid caller's thread id
     */
    bool relocate(int table_idx, int slot_idx, int tid);
};

#endif
