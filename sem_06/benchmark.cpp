#include <benchmark/benchmark.h>
#include <unistd.h>

#include <cmath>
#include <functional>
#include <random>

#include "matmul.h"

static void BM_matmul(benchmark::State &state, algorithm alg) {
    matrix::size_type n = state.range(0) + 1;
    std::mt19937 gen(0);
    std::normal_distribution<> d(0.0, 1 / std::sqrt(n));

    matrix lhs(boost::extents[n][n]);
    matrix rhs(boost::extents[n][n]);
    for (matrix::index i = 0; i < (matrix::index)n; ++i) {
        for (matrix::index j = 0; j < (matrix::index)n; ++j) {
            lhs[i][j] = d(gen);
            rhs[i][j] = d(gen);
        }
    }

    for (auto _ : state) {
        matrix res = matmul(lhs, rhs, alg);
        benchmark::DoNotOptimize(res.data());
    }
}
BENCHMARK_CAPTURE(BM_matmul, naive, algorithm::naive)
    ->RangeMultiplier(2)
    ->Range(1 << 2, 1 << 10);
BENCHMARK_CAPTURE(BM_matmul, transpose, algorithm::transpose)
    ->RangeMultiplier(2)
    ->Range(1 << 2, 1 << 10);
BENCHMARK_CAPTURE(BM_matmul, divide_and_conquer, algorithm::divide_and_conquer)
    ->RangeMultiplier(2)
    ->Range(1 << 2, 1 << 10);
BENCHMARK_CAPTURE(BM_matmul, tiled, algorithm::tiled)
    ->RangeMultiplier(2)
    ->Range(1 << 2, 1 << 10);

BENCHMARK_MAIN();
