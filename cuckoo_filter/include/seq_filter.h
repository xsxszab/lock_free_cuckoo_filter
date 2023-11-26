#ifndef SEQ_FILTER_H
#define SEQ_FILTER_H

// maximum number of items can be stored in one hash table entry
#define NUM_ITEMS_PER_ENTRY 4

#include <array>
#include <cstdint>
#include <string>
#include <vector>

class SequentialFilter {
   public:
    SequentialFilter(int capacity);
    ~SequentialFilter();

    bool insert(const std::string& key);
    bool find(const std::string& key);
    bool remove(const std::string& key);

   private:
    int table_size;
    std::vector<std::array<std::string, NUM_ITEMS_PER_ENTRY>> hash_table;
};

#endif