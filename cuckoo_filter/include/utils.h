#ifndef UTILS_H
#define UTILS_H

/**
 * @file utils.h
 * @brief this file contains some utility functions used by the cuckoo filter
 */

#include <string>

/**
 * @brief generate a random string, possible characters are 0-9, a-z and A-Z.
 * @param[in] len the length of the string
 * @return the generated string
 */
std::string gen_random_string(const int len);

#endif
