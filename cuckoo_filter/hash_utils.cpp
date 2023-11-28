#include <hash_utils.h>

// a modified version of OpenSSL example code
// https://www.openssl.org/docs/manmaster/man3/EVP_Digest.html
std::string md5_fingerprint(const std::string& key) {
    EVP_MD_CTX* mdctx;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    // const EVP_MD* ptr = EVP_get_digestbyname("MD5");
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit(mdctx, EVP_md5());
    EVP_DigestUpdate(mdctx, (const void*)key.c_str(), key.size());
    EVP_DigestFinal_ex(mdctx, md_value, &md_len);
    EVP_MD_CTX_free(mdctx);

    return std::string((char*)md_value, md_len);
}

// hash function, string -> unsigned int32. Note that in order to use this
// function in a hash table, the hash table index should be 'hash % table_size'
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

// not used
uint32_t hash(uint32_t x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

uint32_t hash(const std::string& key) { return jenkins_hash(key); }
