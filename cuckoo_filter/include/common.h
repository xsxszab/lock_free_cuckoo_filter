#ifndef COMMON_H
#define COMMON_H

/**
 * @file common.h
 * @brief this header file defines some variables used by cuckoo filters.
 */

/**
 * @brief maximum number of items can be stored in one hash table entry
 */
#define NUM_ITEMS_PER_ENTRY 4

/**
 * @brief  maximum number of replacement can happen for inserting one key. if
 * this number is exceeded, the hash table is considered full and the insertion
 * operation will fail.
 */
#define NUM_MAX_KICKS 10

#endif
