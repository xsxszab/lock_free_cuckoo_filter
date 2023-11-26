#include <cstdint>
#include <string>

uint32_t jenkins_hash(const std::string& key) {
    int i = 0;
    const int length = key.size();
    uint32_t hash_val = 0;
    while (i != length) {
        hash_val += key[i++];
        hash_val += hash_val << 10;
        hash_val ^= hash_val >> 6;
    }
    hash_val += hash_val << 3;
    hash_val ^= hash_val >> 11;
    hash_val += hash_val << 15;
    return hash_val;
}
