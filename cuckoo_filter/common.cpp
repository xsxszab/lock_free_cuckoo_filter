#include <common.h>

std::string gen_random_string(const int len) {
    static const char chars[] =
        "01234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string ret;
    ret.reserve(len);

    for (int i = 0; i < len; i++) {
        ret += chars[rand() % sizeof(chars) - 1];
    }

    return ret;
}
