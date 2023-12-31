#ifndef POINTER_UTILS_H
#define POINTER_UTILS_H

#include <cstdint>

/**
 * @file pointer_utils.h
 * @brief this header file defines the pointer type used in the lock-free cuckoo
 * filter and its related operations
 */

/**
 * @brief a table pointer is a real 64bit pointer populated with additional
 * counter and mark information.
 *
 * The table pointer's highest 16bits are used by
 * the counter, the remaining 48bits are used by the real pointer (currently x64
 * platforms only use the lower 48 bits for storing pointers). Due to address
 * alignment, the Least Significant Bit of the pointer will always be 0, here
 * the LSB is used to contain a one-bit mark.
 */
using table_pointer = uint64_t;

/**
 * @brief 63rd...48th bits in a 64bit integer
 */
static const uint64_t counter_mask = 0xFFFF000000000000;

/**
 * @brief 47th...1st bits in a 64bit integer
 */
static const uint64_t real_pointer_mask = 0xFFFFFFFFFFFE;

/**
 * @brief 0th bit in a 64bit integer
 */
static const uint64_t mark_mask = 0x1;

/**
 * @brief  get the counter value stored in the table_pointer
 * @param[in] pointer a table_pointer
 * @return counter value
 *
 */
uint16_t get_counter(const table_pointer pointer);

/**
 * @brief update the table pointer using new counter value (in-place)
 * @param[in, out] pointer the table pointer to be updated
 * @param[in] counter_val the new counter value. Not that valid counter value is
 * 0~65535.
 */
void update_counter(table_pointer& pointer, const uint64_t counter_val);

/**
 * @brief create a table pointer using new counter value (does not modify
 * original pointer)
 * @param[in] pointer a table_pointer
 * @param[in] counter_val new counter value
 * @return a table_pointer with the new counter value
 */
table_pointer get_new_counter(const table_pointer pointer,
                              const uint64_t counter_val);

/**
 * @brief get the real pointer stored in the table_pointer
 * @param[in] pointer a table pointer
 * @return the real pointer
 */
uint64_t get_real_pointer(const table_pointer pointer);

/**
 * @brief update the table pointer using new real pointer (in-place)
 * @param[in, out] pointer the table_pointer to be modified.
 * @param[in] real_pointer the new real pointer
 */
void update_real_pointer(table_pointer& pointer, const uint64_t real_pointer);

/**
 * @brief create a table pointer using new real pointer (does not modify
 * original pointer)
 * @param[in] pointer a table pointer
 * @param[in] real_pointer the new real pointer
 * @return a table_pointer with new real pointer value
 */
table_pointer get_new_real_pointer(const table_pointer pointer,
                                   const uint64_t real_pointer);

/**
 * @brief get the mark stored in the table_pointer.
 * @param[in] pointer a table_pointer
 * @return the mark
 */
bool get_marked(const table_pointer pointer);

/**
 * @brief update the table pointer using new mark (in-place)
 * @param[in, out] pointer the table_pointer to be updated
 * @param[in] marked the new mark value
 */
void update_marked(table_pointer& pointer, const bool marked);

/**
 * @brief create a table pointer using new mark (does not modify original
 * pointer).
 * @param[in] pointer a table_pointer
 * @param[in] marked the new mark value
 * @return a table_pointer with new mark value
 */
table_pointer get_new_mark(const table_pointer pointer, const bool marked);

/**
 * @brief create a new table pointer. counter_val should be 0~65535
 * @param[in] counter the counter
 * @param[in] real_pointer the real pointer
 * @param[in] marked the mark value
 */
table_pointer create_pointer(const uint64_t counter,
                             const uint64_t real_pointer, bool marked);

/**
 * @brief print out the pointer in bit prepresentation
 * @param[in] pointer a table_pointer
 */
void print_pointer(const table_pointer pointer);

#endif
