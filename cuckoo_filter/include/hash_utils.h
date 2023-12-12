#ifndef HASH_UTILS_H
#define HASH_UTILS_H

/**
 * @file hash_utils.h
 * @brief this header file contains implementaions of fingerprint and hashing
 * algorithms used by the cuckoo filter.
 */

// for md5 fingerprinting
#include <openssl/evp.h>

#include <cstdint>
#include <string>

/**
 * @brief calculate a string's fingerprint. This function is a modified version
 * of OpenSSL's example code
 * (https://www.openssl.org/docs/manmaster/man3/EVP_Digest.html).
 *
 * @param[in] key the input string
 * @return the key's fingerprint.
 */
std::string md5_fingerprint(const std::string& key);

// hash function, string -> unsigned int32. Note that in order to use this
// function in a hash table, the hash table index should be 'hash % table_size'

/**
 * @brief jenkins hash function. modified based on code from
 * https://en.wikipedia.org/wiki/Jenkins_hash_function .
 * @param[in] key the input key
 * @return the hash value
 */
uint32_t jenkins_hash(const std::string& key);

/**
 * @brief not used
 */
uint32_t hash(uint32_t x);

/**
 * @brief not used
 */
uint32_t hash(const std::string& key);

#endif
