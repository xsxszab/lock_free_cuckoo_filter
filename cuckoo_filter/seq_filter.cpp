#include <hash_utils.h>
#include <seq_filter.h>

#include <iostream>

SequentialFilter::SequentialFilter(int capacity) {
    table_size = capacity;
    hash_table.resize(capacity);
    verbose = false;
}

SequentialFilter::SequentialFilter(int capacity, bool _verbose) {
    table_size = capacity;
    hash_table.resize(capacity);
    verbose = _verbose;
    if (verbose) {
        std::cout << "sequantial filter started" << std::endl;
    }
}

SequentialFilter::~SequentialFilter() {
    if (verbose) {
        std::cout << "sequantial filter terminated" << std::endl;
    }
}

bool SequentialFilter::insert(const std::string& key) {
    mtx.lock();
    if (verbose) {
        std::cout << "try to insert key " << key << " into filter" << std::endl;
    }
    std::string fingerprint = md5_fingerprint(key);
    uint32_t hash1 = jenkins_hash(key) % table_size;
    uint32_t hash2 = hash1 ^ (jenkins_hash(fingerprint) % table_size);

    for (int i = 0; i < NUM_ITEMS_PER_ENTRY; i++) {
        if (hash_table[hash1][i].empty()) {
            hash_table[hash1][i] = fingerprint;
            if (verbose) {
                std::cout << "find empty slot at entry " << hash1 << ", index "
                          << i << std::endl;
            }
            mtx.unlock();
            return true;
        }
    }

    for (int i = 0; i < NUM_ITEMS_PER_ENTRY; i++) {
        if (hash_table[hash2][i].empty()) {
            hash_table[hash2][i] = fingerprint;
            if (verbose) {
                std::cout << "find empty slot at entry " << hash2 << ", index "
                          << i << std::endl;
            }
            mtx.unlock();
            return true;
        }
    }

    int flag = rand() % 2;
    uint32_t replace_hash = flag == 0 ? hash1 : hash2;

    // replace phase
    if (verbose) {
        std::cout << "cannot find empty slot, start replacement" << std::endl;
    }
    for (int n = 0; n < NUM_MAX_KICKS; n++) {
        int bucket_idx = rand() % NUM_ITEMS_PER_ENTRY;
        // randomly pick a stored fingerprint to replace
        std::swap(fingerprint, hash_table[replace_hash][bucket_idx]);
        replace_hash ^= jenkins_hash(fingerprint) % table_size;
        for (int i = 0; i < NUM_ITEMS_PER_ENTRY; i++) {
            if (hash_table[replace_hash][i].empty()) {
                hash_table[replace_hash][i] = fingerprint;
                std::cout << "found replacement location" << std::endl;
                mtx.unlock();
                return true;
            }
        }
    }
    if (verbose) {
        std::cout << "hash table capacity too low" << std::endl;
    }
    mtx.unlock();
    return false;
}

bool SequentialFilter::find(const std::string& key) {
    mtx.lock();
    if (verbose) {
        std::cout << "try to find key " << key << std::endl;
    }
    std::string fingerprint = md5_fingerprint(key);
    uint32_t hash1 = jenkins_hash(key) % table_size;
    uint32_t hash2 = hash1 ^ (jenkins_hash(fingerprint) % table_size);

    for (int i = 0; i < NUM_ITEMS_PER_ENTRY; i++) {
        if (hash_table[hash1][i] == fingerprint) {
            if (verbose) {
                std::cout << "found key " << key << "at entry " << hash1
                          << ", index " << i << std::endl;
            }
            mtx.unlock();
            return true;
        }
    }

    for (int i = 0; i < NUM_ITEMS_PER_ENTRY; i++) {
        if (hash_table[hash2][i] == fingerprint) {
            if (verbose) {
                std::cout << "found key " << key << "at entry " << hash2
                          << ", index " << i << std::endl;
            }
            mtx.unlock();
            return true;
        }
    }
    if (verbose) {
        std::cout << "cannot find key " << key << std::endl;
    }
    mtx.unlock();
    return false;
}

bool SequentialFilter::remove(const std::string& key) {
    mtx.lock();
    if (verbose) {
        std::cout << "try to remove key " << key << std::endl;
    }
    std::string fingerprint = md5_fingerprint(key);
    uint32_t hash1 = jenkins_hash(key) % table_size;
    uint32_t hash2 = hash1 ^ (jenkins_hash(fingerprint) % table_size);

    for (int i = 0; i < NUM_ITEMS_PER_ENTRY; i++) {
        if (hash_table[hash1][i].length() != 0) {
            hash_table[hash1][i].clear();
            if (verbose) {
                std::cout << "removed key " << key << "at entry " << hash1
                          << ", index " << i << std::endl;
            }
            mtx.unlock();
            return true;
        }
    }

    for (int i = 0; i < NUM_ITEMS_PER_ENTRY; i++) {
        if (hash_table[hash2][i].length() != 0) {
            hash_table[hash2][i].clear();
            if (verbose) {
                std::cout << "removed key " << key << "at entry " << hash2
                          << ", index " << i << std::endl;
            }
            mtx.unlock();
            return true;
        }
    }
    // yes, no verbose condition here
    std::cout << "warning, trying to delete non-exsitent key, the filter's "
                 "behavior is undefined"
              << std::endl;
    mtx.unlock();
    return false;
}

int SequentialFilter::size() const { return table_size; }
