#include <seq_filter.h>

int main() {
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
    return 0;
}
