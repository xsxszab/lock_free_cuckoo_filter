#include <pointer_utils.h>

#include <iostream>

void check_pointer(const table_pointer ptr) {
    print_pointer(ptr);
    std::cout << "pointer: " << (int *)get_real_pointer(ptr) << std::endl;
    std::cout << "pointing to value " << *(int *)get_real_pointer(ptr)
              << std::endl;
    std::cout << "counter: " << get_counter(ptr) << std::endl;
    std::cout << "marked " << get_marked(ptr) << std::endl;
    std::cout << "---------------------------" << std::endl;
}

int main() {
    int *a = new int(4);
    table_pointer ptr1 = create_pointer(UINT16_MAX, (uint64_t)a, 1);
    check_pointer(ptr1);
    table_pointer ptr2 = get_new_counter(ptr1, 16);
    update_counter(ptr1, 16);
    check_pointer(ptr1);
    check_pointer(ptr2);

    int *b = new int(1234);
    ptr2 = get_new_real_pointer(ptr1, (uint64_t)b);
    update_real_pointer(ptr1, (uint64_t)b);
    check_pointer(ptr1);
    check_pointer(ptr2);

    ptr2 = get_new_mark(ptr1, false);
    update_marked(ptr1, false);
    check_pointer(ptr1);
    check_pointer(ptr2);

    delete a;
    delete b;
    return 0;
}
