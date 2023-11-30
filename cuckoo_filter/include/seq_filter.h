#ifndef SEQ_FILTER_H
#define SEQ_FILTER_H

#include <common.h>
#include <marcos.h>

#include <array>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

class SequentialFilter {
   public:
    // capacity: number of entries the table should have
    SequentialFilter(int capacity);
    // if verbose is set to true, the filter will print out debug information
    SequentialFilter(int capacity, bool _verbose);

    // self-explanatory
    DISABLE_COPY_AND_MOVE_CONSTRUCT(SequentialFilter)

    ~SequentialFilter();

    // insert a key into the hash table. Return true if successfully inserted
    // new key, false is the insertion failed (hash table full)
    bool insert(const std::string& key);
    // find is a given key is present in the hash table. Return true if present,
    // false if not.
    bool find(const std::string& key);

    // remove a key from the hash table. Return true if the key is deleted,
    // false if the key is not present in the table. Note that when using this
    // filter, it is the user's responsibility to ensure that any key must be in
    // the table before deletion, i.e., this function must always return true,
    // otherwise the filter's behavior is undefined.
    bool remove(const std::string& key);

    // return the hash table's size (number of entries, not the table_size *
    // NUM_ITEMS_PER_ENTRY)
    int size() const;

   private:
    // how many entries are in the hash table, note that one entry contains
    // NUM_ITEMS_PER_ENTRY item slots.
    int table_size;
    // true if print out debug information, false if not
    bool verbose;
    std::vector<std::array<std::string, NUM_ITEMS_PER_ENTRY>> hash_table;

    // mutex for concurrent table access
    std::mutex mtx;
};

#endif