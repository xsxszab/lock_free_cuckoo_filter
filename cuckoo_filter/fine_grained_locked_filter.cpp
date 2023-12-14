#include <hash_utils.h>
#include <fine_grained_locked_filter.h>

#include <iostream>


FineGrainedFilter::FineGrainedFilter(int capacity) {
    table_size = capacity;
    hash_table.resize(capacity);
    verbose = false;
}

FineGrainedFilter::FineGrainedFilter(int capacity, bool _verbose) {
    table_size = capacity;
    hash_table.resize(capacity);
    verbose = _verbose;
    if (verbose) {
        std::cout << "sequantial filter started" << std::endl;
    }
}

FineGrainedFilter::~FineGrainedFilter() {
    if (verbose) {
        std::cout << "sequantial filter terminated" << std::endl;
    }
}

bool FineGrainedFilter::insert(const std::string& key) {
    if (verbose) {
        std::cout << "try to insert key " << key << " into filter" << std::endl;
    }
    std::string fingerprint = md5_fingerprint(key);
    uint32_t hash1 = jenkins_hash(key) % table_size;
    uint32_t hash2 = get_next_hash_index(hash1, fingerprint);

    std::unique_lock<std::shared_mutex> lock1(mutexes[hash1 % mutexes.size()]);
    for (int i = 0; i < NUM_ITEMS_PER_ENTRY; i++) {
        if (hash_table[hash1][i].empty()) {
            hash_table[hash1][i] = fingerprint;
            if (verbose) {
                std::cout << "find empty slot at entry " << hash1 << ", index "
                          << i << std::endl;
            }
            return true;
        }
    }
    lock1.unlock();

    std::unique_lock<std::shared_mutex> lock2(mutexes[hash2 % mutexes.size()]);
    for (int i = 0; i < NUM_ITEMS_PER_ENTRY; i++) {
        if (hash_table[hash2][i].empty()) {
            hash_table[hash2][i] = fingerprint;
            if (verbose) {
                std::cout << "find empty slot at entry " << hash2 << ", index "
                          << i << std::endl;
            }
            return true;
        }
    }
    lock2.unlock();

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
        replace_hash = get_next_hash_index(replace_hash, fingerprint);

        std::unique_lock<std::shared_mutex> lock_replace(mutexes[replace_hash % mutexes.size()]);
        for (int i = 0; i < NUM_ITEMS_PER_ENTRY; i++) {
            if (hash_table[replace_hash][i].empty()) {
                hash_table[replace_hash][i] = fingerprint;
                if (verbose) {
                    std::cout << "found replacement location" << std::endl;
                }
                return true;
            }
        }
        lock_replace.unlock();
    }

    if (verbose) {
        std::cout << "hash table capacity too low" << std::endl;
    }
    return false;
}

int FineGrainedFilter::get_next_hash_index(const int curr_idx,
                                          const std::string fingerprint) {
    return (curr_idx ^ (jenkins_hash(fingerprint) % table_size)) % table_size;
}

bool FineGrainedFilter::find(const std::string& key) {
    if (verbose) {
        std::cout << "try to find key " << key << std::endl;
    }
    std::string fingerprint = md5_fingerprint(key);
    uint32_t hash1 = jenkins_hash(key) % table_size;
    uint32_t hash2 = get_next_hash_index(hash1, fingerprint);

    std::shared_lock<std::shared_mutex> lock1(mutexes[hash1 % mutexes.size()]);
    for (int i = 0; i < NUM_ITEMS_PER_ENTRY; i++) {
        if (hash_table[hash1][i] == fingerprint) {
            if (verbose) {
                std::cout << "found key " << key << "at entry " << hash1
                          << ", index " << i << std::endl;
            }
            return true;
        }
    }
    lock1.unlock();

    std::shared_lock<std::shared_mutex> lock2(mutexes[hash2 % mutexes.size()]);
    for (int i = 0; i < NUM_ITEMS_PER_ENTRY; i++) {
        if (hash_table[hash2][i] == fingerprint) {
            if (verbose) {
                std::cout << "found key " << key << "at entry " << hash2
                          << ", index " << i << std::endl;
            }
            return true;
        }
    }
    lock2.unlock();

    if (verbose) {
        std::cout << "cannot find key " << key << std::endl;
    }
    return false;
}

bool FineGrainedFilter::remove(const std::string& key) {
    if (verbose) {
        std::cout << "try to remove key " << key << std::endl;
    }
    std::string fingerprint = md5_fingerprint(key);
    uint32_t hash1 = jenkins_hash(key) % table_size;
    uint32_t hash2 = get_next_hash_index(hash1, fingerprint);

    std::unique_lock<std::shared_mutex> lock1(mutexes[hash1 % mutexes.size()]);
    for (int i = 0; i < NUM_ITEMS_PER_ENTRY; i++) {
        if (hash_table[hash1][i].length() != 0 &&
            hash_table[hash1][i] == fingerprint) {
            hash_table[hash1][i].clear();
            if (verbose) {
                std::cout << "removed key " << key << "at entry " << hash1
                          << ", index " << i << std::endl;
            }
            return true;
        }
    }
    lock1.unlock();

    std::unique_lock<std::shared_mutex> lock2(mutexes[hash2 % mutexes.size()]);
    for (int i = 0; i < NUM_ITEMS_PER_ENTRY; i++) {
        if (hash_table[hash2][i].length() != 0 &&
            hash_table[hash2][i] == fingerprint) {
            hash_table[hash2][i].clear();
            if (verbose) {
                std::cout << "removed key " << key << "at entry " << hash2
                          << ", index " << i << std::endl;
            }
            return true;
        }
    }
    lock2.unlock();

    std::cout << "warning, trying to delete non-exsitent key, the filter's "
                 "behavior is undefined"
              << std::endl;

    return false;
}

int FineGrainedFilter::size() const { return table_size; }
