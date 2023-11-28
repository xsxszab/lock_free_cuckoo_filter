#include <common.h>
#include <seq_filter.h>

#include <thread>

#define NUM_THREADS 8
#define NUM_STRINGS_PER_THREAD 256
#define STRING_LENGTH 40

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
    SequentialFilter filter(4096, true);
    std::vector<std::string> strings;

    for (int i = 0; i < NUM_THREADS * NUM_STRINGS_PER_THREAD; i++) {
        strings.push_back(gen_random_string(STRING_LENGTH));
    }

    return 0;
}
