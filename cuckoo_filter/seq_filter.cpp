#include <hash_utils.h>
#include <seq_filter.h>

SequentialFilter::SequentialFilter(int capacity) {
    table_size = capacity;
    hash_table.resize(capacity);
}

SequentialFilter::~SequentialFilter() {}

bool SequentialFilter::insert(const std::string& key) {
    std::string fingerprint = md5_fingerprint(key);
    uint32_t hash1 = jenkins_hash(key) % table_size;
    uint32_t hash2 = hash1 ^ (jenkins_hash(fingerprint) % table_size);

    for (int i = 0; i < NUM_ITEMS_PER_ENTRY; i++) {
    }

    for (int i = 0; i < NUM_ITEMS_PER_ENTRY; i++) {
    }
}

bool SequentialFilter::find(const std::string& key) {
    std::string fingerprint = md5_fingerprint(key);
    uint32_t hash1 = jenkins_hash(key) % table_size;
    uint32_t hash2 = hash1 ^ (jenkins_hash(fingerprint) % table_size);
    return false;
}

bool SequentialFilter::remove(const std::string& key) {
    std::string fingerprint = md5_fingerprint(key);
    uint32_t hash1 = jenkins_hash(key) % table_size;
    uint32_t hash2 = hash1 ^ (jenkins_hash(fingerprint) % table_size);
    return false;
}
