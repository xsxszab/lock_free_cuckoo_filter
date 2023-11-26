#ifndef HASH_UTILS_H
#define HASH_UTILS_H

#include <openssl/evp.h>

#include <cstdint>
#include <string>

// a modified version of OpenSSL example code
// https://www.openssl.org/docs/manmaster/man3/EVP_Digest.html
std::string md5_fingerprint(const std::string& key);

// hash function, string -> unsigned int32. Note that in order to use this
// function in a hash table, the hash table index should be 'hash % table_size'
uint32_t jenkins_hash(const std::string& key);

// not used
uint32_t hash(uint32_t x);

#endif
