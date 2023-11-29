#include <pointer_utils.h>

#include <bitset>
#include <iostream>

uint16_t get_counter(const table_pointer pointer) {
    return (pointer & counter_mask) >> 48;
}

void update_counter(table_pointer& pointer, const uint64_t counter_val) {
    if (counter_val > UINT16_MAX) {
        std::cout << "[ERROR] counter value too large" << std::endl;
        abort();
    }
    pointer &= ~counter_mask;
    pointer |= counter_val << 48;
}

table_pointer get_new_counter(const table_pointer pointer,
                              const uint64_t counter_val) {
    if (counter_val > UINT16_MAX) {
        std::cout << "[ERROR] counter value too large" << std::endl;
        abort();
    }
    table_pointer ret = pointer;
    ret &= ~counter_mask;
    ret |= counter_val << 48;
    return ret;
}

uint64_t get_real_pointer(const table_pointer pointer) {
    return pointer & real_pointer_mask;
}

void update_real_pointer(table_pointer& pointer, const uint64_t real_pointer) {
    pointer &= ~real_pointer_mask;
    pointer |= real_pointer;
}

table_pointer get_new_real_pointer(const table_pointer pointer,
                                   const uint64_t real_pointer) {
    table_pointer ret = pointer;
    ret &= ~real_pointer_mask;
    ret |= real_pointer;
    return ret;
}

bool get_marked(const table_pointer pointer) { return pointer & mark_mask; }

void update_marked(table_pointer& pointer, const bool marked) {
    pointer &= ~mark_mask;
    pointer |= (uint64_t)marked;
}

table_pointer get_new_mark(const table_pointer pointer, const bool marked) {
    table_pointer ret = pointer;
    ret &= ~mark_mask;
    ret |= marked;
    return ret;
}

table_pointer create_pointer(const uint64_t counter,
                             const uint64_t real_pointer, bool marked) {
    if (counter > UINT16_MAX) {
        std::cout << "[ERROR] counter value too large" << std::endl;
        abort();
    }
    table_pointer ret = 0x0;
    ret |= marked;
    ret |= real_pointer;
    ret |= counter << 48;
    return ret;
}

void print_pointer(const table_pointer pointer) {
    std::bitset<64> tmp(pointer);
    std::cout << tmp << std::endl;
}
