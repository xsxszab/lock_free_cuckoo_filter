#include <cycle_timer.h>
#include <fine_grained_locked_filter.h>
#include <lock_free_filter.h>
#include <seq_filter.h>

#include <iostream>

#define HASH_TABLE_SIZE 256000
#define STRING_LENGTH 256

#define NUM_THREADS 32

int main() {
    SequentialFilter seq_filter(HASH_TABLE_SIZE, false);

    LockFreeCuckooFilter lock_free_filter(HASH_TABLE_SIZE, NUM_THREADS, false);

    return 0;
}
