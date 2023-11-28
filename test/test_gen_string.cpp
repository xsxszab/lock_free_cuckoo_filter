#include <common.h>

#include <iostream>

int main() {
    for (int i = 0; i < 100; i++) {
        std::cout << gen_random_string(40) << std::endl;
    }
    return 0;
}
