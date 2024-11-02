#include <unistd.h>

#include <iostream>
#include <thread>
#include <vector>

constexpr int N_REPS = 1'000'000'000;
static size_t cache_line_sz = sysconf(_SC_LEVEL3_CACHE_LINESIZE);
static size_t ints_per_cache_line = cache_line_sz / sizeof(int);

int main() {
    std::vector<int> two_lines(2 * ints_per_cache_line);

    std::thread thread1([&two_lines] {
        for (int i = 0; i < N_REPS; ++i) two_lines[0] = i;
        // | i | 0 |
    });

    std::thread thread2([&two_lines] {
        for (int i = 0; i < N_REPS; ++i) two_lines[1] = i;
        // | 0 | i |
    });

    thread1.join();
    thread2.join();
    std::cout << two_lines[0] << std::endl;
    return 0;
}
