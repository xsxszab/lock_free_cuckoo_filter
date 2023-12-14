#ifndef FINE_GRAINED_LOCKED_FILTER_H
#define FINE_GRAINED_LOCKED_FILTER_H

/**
 * @file fine_grained_locked_filter.h
 * @brief this header file contains implementaions of a fine-grained locked
 * cuckoo filter.
 */

#include <common.h>
#include <marcos.h>

#include <array>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>
#include <condition_variable>
#include <thread>
#include <unordered_map>
#include <shared_mutex>


/**
 * @brief A coarse-grained locked cuckoo filter implementation.
 */
class FineGrainedFilter {
   public:
    /**
     * @brief construct a coarse grained locked cuckoo filter instance
     * @param[in] capacity size of the hash table
     * concurrently.
     */
    FineGrainedFilter(int capacity);

    /**
     * @brief construct a coarse grained locked cuckoo filter instance
     * @param[in] capacity size of the hash table
     * concurrently.
     * @param[in] _verbose Whether or not to print out debug information.
     */
    FineGrainedFilter(int capacity, bool _verbose);

    // self-explanatory
    DISABLE_COPY_AND_MOVE_CONSTRUCT(FineGrainedFilter)

    /**
     * @brief destory the fileter instance
     */
    ~FineGrainedFilter();

    /**
     * @brief insert a key into the hash table
     * @param[in] key the key to be inserted
     * @return true if successfully inserted the key, false if failed (hash
     * table full).
     */
    bool insert(const std::string& key);

    /**
     * @brief find if a given key is present in the hash table. Return true if
     * present, false if not.
     * @param[in]  key The key to search.
     *
     * @return true if found, false if not found.
     */
    bool find(const std::string& key);

    /**
     * @brief Remove a key from the hash table.
     * @param[in] key The key to delete.
     *
     * @return true if successfully deleted the key, false if not. Note that it
     * is the user's responsibility to ensure that any key must be present
     * before deletion, i.e., this function must always return true, otherwise
     * the filter's behavior is undefined.
     */
    bool remove(const std::string& key);

    /**
     * @brief get the hash table's size
     * @return the hash table's size
     */
    int size() const;

   private:
    /**
     * @brief  how many entries are in the hash table, note that one entry
     * contains NUM_ITEMS_PER_ENTRY item slots.
     */
    int table_size;

    /**
     * @brief true if print out debug information, false if not
     */
    bool verbose;

    /**
     * @brief the internal hash table
     */
    std::vector<std::array<std::string, NUM_ITEMS_PER_ENTRY>> hash_table;

    /**
     * @brief calculate a key's next hash table index based on the current index
     * and its fingerprint.
     * @param[in] curr_idx current index
     * @param[in] fingerprint the key's MD5 fingerprint
     */
    int get_next_hash_index(const int curr_idx, const std::string fingerprint);

    /**
     * @brief mutexes for concurrent table access
     */
    std::vector<std::shared_mutex> mutexes{8};
};

#endif

