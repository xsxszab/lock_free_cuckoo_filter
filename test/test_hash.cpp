#include <hash_utils.h>

#include <iostream>

int main() {
    std::string test_str_1("a");
    std::string test_str_2("foo bar");
    std::string test_str_3(
        "fjdsaojfdoiasiojbiosajdiogsofi;adjfgbsoifagdlhbiosdf");

    std::cout << md5_fingerprint(test_str_1) << " "
              << jenkins_hash(md5_fingerprint(test_str_1)) << std::endl;
    std::cout << md5_fingerprint(test_str_2) << " "
              << jenkins_hash(md5_fingerprint(test_str_2)) << std::endl;
    std::cout << md5_fingerprint(test_str_3) << " "
              << jenkins_hash(md5_fingerprint(test_str_3)) << std::endl;
    return 0;
}
