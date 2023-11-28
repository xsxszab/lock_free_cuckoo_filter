#include <hash_utils.h>

#include <iostream>

int main() {
    std::string str1(
        "0000000000000000000000000000000000000000000000000000000000000000");
    std::string str2(
        "0000000000000000000000000000000000000000000000000000000000000000");
    std::string str3(
        "000000000000000000000000000000000000000000000000000000000000000012");
    std::string str4(
        "00000000000000000000000000000000000000000000000000000000000000001234");

    std::cout << md5_fingerprint(str1) << std::endl;
    std::cout << md5_fingerprint(str2) << std::endl;
    std::cout << md5_fingerprint(str3) << std::endl;
    std::cout << md5_fingerprint(str4) << std::endl;
    return 0;
}
