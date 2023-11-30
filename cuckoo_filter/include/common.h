
// this header file defines some variables used by cuckoo filters.

// maximum number of items can be stored in one hash table entry
#define NUM_ITEMS_PER_ENTRY 4

// maximum number of replacement can happen for inserting one key, if this
// number is exceeded, the hash table is considered full and the insertion will
// fail.
#define NUM_MAX_KICKS 10
