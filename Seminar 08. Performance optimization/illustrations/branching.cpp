#include <iostream>
#include <random>

constexpr int N_REPS = 1'000'000'000;

int main() {
    int a = 0;
    std::mt19937 gen(0);
    std::uniform_int_distribution<> distr(0, 1);

    unsigned val = 0;
    for (int i = 0; i < N_REPS; ++i) {
        // val = distr(gen);
        val = (val + 1) % 5;
        if (val == 0 || val == 2) {
            ++a;
        }
    }

    std::cout << val << ' ' << a << std::endl;
    return 0;
}
