#include <common.h>
#include <seq_filter.h>

#include <iostream>
#include <thread>

#define NUM_THREADS 8
#define NUM_STRINGS_PER_THREAD 512
#define STRING_LENGTH 40
#define NUM_STRINGS (NUM_STRINGS_PER_THREAD * NUM_THREADS)
#define HASH_TABLE_SIZE 2048

int main() {
    // single thread test
    {
        // turn on verbose mode to print out debug information
        SequentialFilter filter(1024, true);
        for (int i = 0; i < NUM_ITEMS_PER_ENTRY * 2; i++) {
            filter.insert("abcdefgh");
        }
        filter.insert("a");
        filter.find("a");
        filter.find("bcadsjofdasjfoidas");
        filter.remove("abcdefgh");
        filter.find("abcdefgh");
    }

    // multithread test
    SequentialFilter filter(HASH_TABLE_SIZE, true);
    std::vector<std::string> strings;

    for (int i = 0; i < NUM_STRINGS; i++) {
        strings.push_back(gen_random_string(STRING_LENGTH));
    }

    auto thread_func_insert = [&](int tid) {
        int str_start_idx = tid * NUM_STRINGS_PER_THREAD;
        int str_end_idx = (tid + 1) * (NUM_STRINGS_PER_THREAD);
        std::cout << "thread " << tid
                  << " started, try to insert strings within range ["
                  << str_start_idx << ", " << str_end_idx << ")" << std::endl;

        for (int i = str_start_idx; i < str_end_idx; i++) {
            filter.insert(strings[i]);
        }
        std::cout << "thread " << tid << " terminated" << std::endl;
    };

    std::thread threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i] = std::thread(thread_func_insert, i);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }

    for (int i = 0; i < NUM_STRINGS; i++) {
        if (!filter.find(strings[i])) {
            std::cout << "error, cannot find string" << strings[i] << std::endl;
        }
    }

    return 0;
}
