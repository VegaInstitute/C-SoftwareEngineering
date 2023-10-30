#pragma once

#include <unistd.h>

#include <algorithm>
#include <boost/multi_array.hpp>
#include <boost/multi_array/multi_array_ref.hpp>

using matrix = boost::multi_array<double, 2>;
using matrix_ref = boost::multi_array_ref<double, 2>;
using const_matrix_ref = boost::const_multi_array_ref<double, 2>;

using range = boost::multi_array_types::index_range;

enum algorithm { naive, transpose, divide_and_conquer, tiled };

template <typename MatLHS, typename MatRHS, typename MatRes>
void matmul_naive(MatLHS lhs, MatRHS rhs, MatRes res) {
    auto nrows = res.shape()[0];
    auto ncols = res.shape()[1];
    auto n_inner = lhs.shape()[1];

    for (matrix::index i = 0; i < (matrix::index)nrows; ++i) {
        for (matrix::index j = 0; j < (matrix::index)ncols; ++j) {
            for (matrix::size_type k = 0; k < n_inner; ++k) {
                res[i][j] += lhs[i][k] * rhs[k][j];
            }
        }
    }
}

template <typename MatLHS, typename MatRHS, typename MatRes>
void matmul_transpose(MatLHS lhs, MatRHS rhs, MatRes res) {
    auto nrows = lhs.shape()[0];
    auto ncols = rhs.shape()[1];
    auto n_inner = lhs.shape()[1];

    matrix rhs_f(boost::extents[n_inner][ncols],
                 boost::fortran_storage_order());

    for (matrix::index i = 0; i < (matrix::index)n_inner; ++i) {
        for (matrix::index j = 0; j < (matrix::index)ncols; ++j) {
            rhs_f[i][j] = rhs[i][j];
        }
    }

    matmul_naive(lhs, rhs_f, res);
}

template <typename MatLHS, typename MatRHS, typename MatRes>
void matmul_tiled(MatLHS lhs, MatRHS rhs, MatRes res) {
    static size_t tile_size =
        sysconf(_SC_LEVEL1_DCACHE_LINESIZE) / sizeof(double);

    auto nrows = lhs.shape()[0];
    auto ncols = rhs.shape()[1];
    auto n_inner = lhs.shape()[1];

    matrix::index i, j, k;
    for (i = 0; i < nrows; i += tile_size) {
        auto i_rem = std::min(nrows - i, tile_size);
        for (j = 0; j < ncols; j += tile_size) {
            auto j_rem = std::min(ncols - j, tile_size);
            for (k = 0; k < n_inner; k += tile_size) {
                auto k_rem = std::min(n_inner - k, tile_size);

                matmul_naive(lhs[boost::indices[range(i, i + i_rem)]
                                               [range(k, k + k_rem)]],
                             rhs[boost::indices[range(k, k + k_rem)]
                                               [range(j, j + j_rem)]],
                             res[boost::indices[range(i, i + i_rem)]
                                               [range(j, j + j_rem)]]);
            }
        }
    }
}

template <typename MatLHS, typename MatRHS, typename MatRes>
void matmul_divide_and_conquer(MatLHS lhs, MatRHS rhs, MatRes res) {
    static size_t stop_split_sz =
        sysconf(_SC_LEVEL1_DCACHE_LINESIZE) / sizeof(double);

    auto nrows = lhs.shape()[0];
    auto ncols = rhs.shape()[1];
    auto n_inner = lhs.shape()[1];

    auto max = std::max({nrows, ncols, n_inner});

    if (max < stop_split_sz) {
        matmul_naive(lhs, rhs, res);
        return;
    }

    if (max == nrows) {
        auto mid = nrows / 2;
        matmul_divide_and_conquer(lhs[boost::indices[range(0, mid)][range()]],
                                  rhs,
                                  res[boost::indices[range(0, mid)][range()]]);
        matmul_divide_and_conquer(
            lhs[boost::indices[range(mid, nrows)][range()]], rhs,
            res[boost::indices[range(mid, nrows)][range()]]);
        return;
    }

    if (max == ncols) {
        auto mid = ncols / 2;
        matmul_divide_and_conquer(lhs,
                                  rhs[boost::indices[range()][range(0, mid)]],
                                  res[boost::indices[range()][range(0, mid)]]);
        matmul_divide_and_conquer(
            lhs, rhs[boost::indices[range()][range(mid, ncols)]],
            res[boost::indices[range()][range(mid, ncols)]]);
        return;
    }

    auto mid = n_inner / 2;
    matmul_divide_and_conquer(lhs[boost::indices[range()][range(0, mid)]],
                              rhs[boost::indices[range(0, mid)][range()]], res);
    matmul_divide_and_conquer(lhs[boost::indices[range()][range(mid, n_inner)]],
                              rhs[boost::indices[range(mid, n_inner)][range()]],
                              res);
}

void matmul_naive_strassen(const_matrix_ref lhs, const_matrix_ref rhs,
                           matrix_ref res);

void matmul(const_matrix_ref lhs, const_matrix_ref rhs, matrix_ref res,
            algorithm alg = algorithm::naive);

matrix matmul(const_matrix_ref lhs, const_matrix_ref rhs,
              algorithm alg = algorithm::naive);
