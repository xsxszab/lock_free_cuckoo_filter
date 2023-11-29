#ifndef POINTER_UTILS_H
#define POINTER_UTILS_H

#include <cstdint>

// a table pointer is a real 64bit pointer populated with additional counter and
// mark information.
using table_pointer = uint64_t;

static const uint64_t counter_mask = 0xFFFF000000000000;
static const uint64_t real_pointer_mask = 0xFFFFFFFFFFFE;
static const uint64_t mark_mask = 0x1;

uint16_t get_counter(const table_pointer pointer);
void update_counter(table_pointer& pointer, const uint64_t counter_val);
table_pointer get_new_counter(const table_pointer pointer,
                              const uint64_t counter_val);

uint64_t get_real_pointer(const table_pointer pointer);
void update_real_pointer(table_pointer& pointer, const uint64_t real_pointer);
table_pointer get_new_real_pointer(const table_pointer pointer,
                                   const uint64_t real_pointer);

bool get_marked(const table_pointer pointer);
void update_marked(table_pointer& pointer, const bool marked);
table_pointer get_new_mark(const table_pointer pointer, const bool marked);

table_pointer create_pointer(const uint64_t counter,
                             const uint64_t read_pointer, bool marked);

void print_pointer(const table_pointer pointer);

#endif
