#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>

#include <boost/multi_array/multi_array_ref.hpp>
#include <functional>
#include <limits>
#include <tuple>

#include "matmul.h"

using ::testing::Combine;
using ::testing::TestWithParam;
using ::testing::Values;

class MatmulParametrized
    : public testing::TestWithParam<std::tuple<
          std::tuple<const_matrix_ref, const_matrix_ref, const_matrix_ref>,
          algorithm> > {};

TEST_P(MatmulParametrized, Result) {
    auto matrices = std::get<0>(GetParam());
    auto alg = std::get<1>(GetParam());

    matrix expected = std::get<2>(matrices);
    matrix product = matmul(std::get<0>(matrices), std::get<1>(matrices), alg);
    auto nrows = product.shape()[0];
    auto ncols = product.shape()[1];
    EXPECT_EQ(nrows, expected.shape()[0]);
    EXPECT_EQ(ncols, expected.shape()[1]);

    for (matrix::index i = 0; i < (matrix::index)nrows; ++i) {
        for (matrix::index j = 0; j < (matrix::index)ncols; ++j) {
            EXPECT_NEAR(product[i][j], expected[i][j],
                        std::numeric_limits<double>::epsilon());
        }
    }
}

constexpr double lhs0[] = {
    0.,
    1.,
    2.,
};
constexpr double rhs0[] = {
    0.,
    1.,
    2.,
};
constexpr double res0[] = {
    0., 0., 0., 0., 1., 2., 0., 2., 4.,
};

constexpr double lhs1[] = {
    0.,
    1.,
    2.,
};
constexpr double rhs1[] = {
    0.,
    1.,
    2.,
};
constexpr double res1[] = {
    5.,
};

INSTANTIATE_TEST_SUITE_P(
    TestMatmuls, MatmulParametrized,
    testing::Combine(
        testing::Values(
            std::make_tuple(
                const_matrix_ref(lhs0, std::array<matrix::index, 2>{{3, 1}}),
                const_matrix_ref(rhs0, std::array<matrix::index, 2>{{1, 3}}),
                const_matrix_ref(res0, std::array<matrix::index, 2>{{3, 3}})),
            std::make_tuple(
                const_matrix_ref(lhs1, std::array<matrix::index, 2>{{1, 3}}),
                const_matrix_ref(rhs1, std::array<matrix::index, 2>{{3, 1}}),
                const_matrix_ref(res1, std::array<matrix::index, 2>{{1, 1}}))),
        testing::Values(algorithm::naive, algorithm::transpose,
                        algorithm::divide_and_conquer, algorithm::tiled)));
