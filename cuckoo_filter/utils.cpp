#include <utils.h>

std::string gen_random_string(const int len) {
    if (len <= 0) {
        abort();
    }
    static const char chars[] =
        "01234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string ret;
    ret.reserve(len);

    for (int i = 0; i < len; i++) {
        ret += chars[rand() % sizeof(chars) - 1];
    }

    return ret;
}
