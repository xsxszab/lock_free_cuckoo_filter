#include <cycle_timer.h>
#include <fine_grained_locked_filter.h>
#include <lock_free_filter.h>
#include <seq_filter.h>
#include <utils.h>

#include <iostream>
#include <thread>

// self-explanatory
#define HASH_TABLE_SIZE 256000
// length of the string inserted into the filter
#define STRING_LENGTH 256

// how many strings will be inserted per thread
#define NUM_STRINGS_PER_THREAD 4000
// self-explanatory
#define NUM_THREADS 32
// total number of strings inserted
#define NUM_STRINGS (NUM_THREADS * NUM_STRINGS_PER_THREAD)

// repeat benchmark for NUM_REPEAT times
#define NUM_REPEAT 3

// for colored test output
const char* GREEN = "\033[0;32m";
const char* NC = "\033[0m";

int main() {
    std::vector<std::string> strings;

    std::cout << GREEN << "Start benchmark" << NC << std::endl;
    std::cout << GREEN << "Number of threads: " << NC << NUM_THREADS
              << std::endl;
    std::cout << GREEN << "Test Scenario: " << NC
              << "90\% Find, 5\% Insert, 5\% Remove, "
              << NUM_STRINGS_PER_THREAD * 20 << " operations in total"
              << std::endl;

    // generate a list of random strings
    for (int i = 0; i < NUM_STRINGS; i++) {
        strings.push_back(gen_random_string(STRING_LENGTH));
    }

    // Coarse-grained locked test start
    double seq_total_time = 0.0;
    for (int i = 0; i < NUM_REPEAT; i++) {
        SequentialFilter seq_filter(HASH_TABLE_SIZE, false);

        auto seq_func = [&](int tid) {
            int str_start_idx = tid * NUM_STRINGS_PER_THREAD;
            int str_end_idx = (tid + 1) * (NUM_STRINGS_PER_THREAD);
            for (int i = str_start_idx; i < str_end_idx; i++) {
                seq_filter.insert(strings[i]);
            }
            for (int i = 0; i < NUM_STRINGS_PER_THREAD * 18; i++) {
                int query_idx = rand() % strings.size();
                seq_filter.find(strings[query_idx]);
            }
            for (int i = str_start_idx; i < str_end_idx; i++) {
                seq_filter.remove(strings[i]);
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
        seq_total_time += end_time - start_time;
    }

    std::cout << "Coarse-grained locked cuckoo filter execution time: "
              << seq_total_time / NUM_REPEAT << "s" << std::endl;

    // Coarse-grained locked test end, Fine-grained locked test start

    double fine_grained_total_time = 0.0;
    for (int i = 0; i < NUM_REPEAT; i++) {
        double start_time = CycleTimer::currentSeconds();
        double end_time = CycleTimer::currentSeconds();
        fine_grained_total_time += end_time - start_time;
    }

    std::cout << "Fine-grained locked cuckoo filter execution time: "
              << fine_grained_total_time / NUM_REPEAT << "s" << std::endl;

    // Fine-grained locked test end, Lock-free test start
    double lock_free_total_time = 0.0;
    for (int i = 0; i < NUM_REPEAT; i++) {
        LockFreeCuckooFilter lock_free_filter(HASH_TABLE_SIZE, NUM_THREADS,
                                              false);

        auto seq_func = [&](int tid) {
            int str_start_idx = tid * NUM_STRINGS_PER_THREAD;
            int str_end_idx = (tid + 1) * (NUM_STRINGS_PER_THREAD);
            for (int i = str_start_idx; i < str_end_idx; i++) {
                lock_free_filter.insert(strings[i], tid);
            }
            for (int i = 0; i < NUM_STRINGS_PER_THREAD * 18; i++) {
                int query_idx = rand() % strings.size();
                lock_free_filter.find(strings[query_idx], tid);
            }
            for (int i = str_start_idx; i < str_end_idx; i++) {
                lock_free_filter.remove(strings[i], tid);
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
        lock_free_total_time += end_time - start_time;
    }

    std::cout << "Lock free cuckoo filter execution time: "
              << lock_free_total_time / NUM_REPEAT << "s" << std::endl;

    // lock free test end
    return 0;
}
