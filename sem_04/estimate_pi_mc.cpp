#include <future>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

double estimate_pi_mc(unsigned n_points) {
    std::mt19937 gen(0);
    std::uniform_real_distribution distr;
    double res = 1.0;

    for (unsigned i = 1; i <= n_points; ++i) {
        double x = distr(gen);
        double y = distr(gen);
        res *= (i - 1.0) / i;
        if (x * x + y * y < 1) {
            res += 1.0 / i;
        }
    }

    res *= 4.0;

    return res;
}

int main() {
    unsigned n_points = 10'000'000;
    // std::cout << estimate_pi_mc(n_points) << '\n';
    unsigned n_threads = 8;

    std::vector<std::jthread> threads;
    std::vector<std::future<double>> futures;
    for (unsigned i = 0; i < n_threads; ++i) {
        std::packaged_task<double(unsigned)> task{estimate_pi_mc};
        futures.emplace_back(task.get_future());
        threads.emplace_back(
            std::jthread(std::move(task), n_points / n_threads));
    }

    double final_res = 0.0;
    for (auto &fut : futures) {
        final_res += fut.get();
    }

    std::cout << final_res / n_threads << '\n';
    return 0;
}
