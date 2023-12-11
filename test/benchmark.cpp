#include <cycle_timer.h>
#include <fine_grained_locked_filter.h>
#include <lock_free_filter.h>
#include <seq_filter.h>
#include <utils.h>

#include <iostream>
#include <thread>

#define HASH_TABLE_SIZE 256000
#define STRING_LENGTH 256

#define NUM_STRINGS_PER_THREAD 4000
#define NUM_THREADS 32
#define NUM_STRINGS (NUM_THREADS * NUM_STRINGS_PER_THREAD)

#define NUM_REPEAT 3

int main() {
    std::vector<std::string> strings;

    // generate a list of random strings
    for (int i = 0; i < NUM_STRINGS; i++) {
        strings.push_back(gen_random_string(STRING_LENGTH));
    }

    // benchmark coarse-grained locked cuckoo filter

    double total_time = 0.0;

    for (int i = 0; i < NUM_REPEAT; i++) {
        SequentialFilter seq_filter(HASH_TABLE_SIZE, false);

        auto seq_func = [&](int tid) {
            int str_start_idx = tid * NUM_STRINGS_PER_THREAD;
            int str_end_idx = (tid + 1) * (NUM_STRINGS_PER_THREAD);
            for (int i = str_start_idx; i < str_end_idx; i++) {
                seq_filter.insert(strings[i]);
            }
        };
        std::thread threads[NUM_THREADS];

        double start_time = CycleTimer::currentSeconds();

        for (int i = 0; i < NUM_THREADS; i++) {
            threads[i] = std::thread(seq_func, i);
        }
        for (int i = 0; i < NUM_THREADS; i++) {
            threads[i].join();
        }

        double end_time = CycleTimer::currentSeconds();
        total_time += end_time - start_time;
    }

    std::cout << "Coarse-grained locked cuckoo filter execution time: "
              << total_time / 3.0 << "s" << std::endl;

    LockFreeCuckooFilter lock_free_filter(HASH_TABLE_SIZE, NUM_THREADS, false);

    // auto lock_free_func = [&](int tid) {
    //     int str_start_idx = tid * NUM_STRINGS_PER_THREAD;
    //     int str_end_idx = (tid + 1) * (NUM_STRINGS_PER_THREAD);
    //     for (int i = str_start_idx; i < str_end_idx; i++) {
    //         lock_free_filter.insert(strings[i], tid);
    //     }
    // };

    return 0;
}
