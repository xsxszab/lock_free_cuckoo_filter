#include <hash_utils.h>

#include <iostream>

int main() {
    std::string test_str_1("a");
    std::string test_str_2("foo bar");
    std::string test_str_3(
        "fjdsaojfdoiasiojbiosajdiogsofi;adjfgbsoifagdlhbiosdf");

    std::cout << jenkins_hash(test_str_1) << std::endl;
    std::cout << jenkins_hash(test_str_2) << std::endl;
    std::cout << jenkins_hash(test_str_3) << std::endl;

    return 0;
}
