#include <lock_free_filter.h>
#include <utils.h>

#include <iostream>
#include <thread>

#define HASH_TABLE_SIZE 4096

#define NUM_STRINGS_PER_THREAD (768 * 16)
#define NUM_THREADS 1

#define NUM_STRINGS (NUM_THREADS * NUM_STRINGS_PER_THREAD)
#define STRING_LENGTH 256

int main() {
    LockFreeCuckooFilter filter(HASH_TABLE_SIZE, NUM_THREADS, true);
    std::vector<std::string> strings;

    // generate a list of random strings
    for (int i = 0; i < NUM_STRINGS; i++) {
        strings.push_back(gen_random_string(STRING_LENGTH));
    }

    // thread function
    auto thread_func_insert = [&](int tid) {
        int str_start_idx = tid * NUM_STRINGS_PER_THREAD;
        int str_end_idx = (tid + 1) * (NUM_STRINGS_PER_THREAD);
        // std::cout << "thread " << tid
        //           << " started, try to insert strings within range ["
        //           << str_start_idx << ", " << str_end_idx << ")" <<
        //           std::endl;

        for (int i = str_start_idx; i < str_end_idx; i++) {
            filter.insert(strings[i], tid);
        }
        // std::cout << "thread " << tid << " terminated" << std::endl;
    };

    auto thread_func_find = [&](int tid) {
        int str_start_idx = tid * NUM_STRINGS_PER_THREAD;
        int str_end_idx = (tid + 1) * (NUM_STRINGS_PER_THREAD);
        // std::cout << "thread " << tid
        //           << " started, try to find strings within range ["
        //           << str_start_idx << ", " << str_end_idx << ")" <<
        //           std::endl;
        for (int i = str_start_idx; i < str_end_idx; i++) {
            if (filter.find(strings[i], tid) == NUM_ITEMS_PER_ENTRY * 2) {
                // std::cout << "error, cannot find string "
                //           << strings[i].substr(0, 16) << std::endl;
            }
        }
    };

    auto thread_func_remove = [&](int tid) {
        int str_start_idx = tid * NUM_STRINGS_PER_THREAD;
        int str_end_idx = (tid + 1) * (NUM_STRINGS_PER_THREAD);
        // std::cout << "thread " << tid
        //           << " started, try to remove strings within range ["
        //           << str_start_idx << ", " << str_end_idx << ")" <<
        //           std::endl;
        for (int i = str_start_idx; i < str_end_idx; i++) {
            filter.remove(strings[i], tid);
        }
    };

    std::thread threads_insert[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        threads_insert[i] = std::thread(thread_func_insert, i);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        threads_insert[i].join();
    }

    std::cout << "string insert finished" << std::endl;

    std::thread threads_find[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        threads_find[i] = std::thread(thread_func_find, i);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        threads_find[i].join();
    }

    std::cout << "string find finished" << std::endl;

    std::thread threads_remove[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        threads_remove[i] = std::thread(thread_func_remove, i);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        threads_remove[i].join();
    }

    std::cout << "string remove finished" << std::endl;

    return 0;
}
