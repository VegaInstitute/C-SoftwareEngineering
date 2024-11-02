#include "matmul.h"

void matmul(const_matrix_ref lhs, const_matrix_ref rhs, matrix_ref res,
            algorithm alg) {
    if (lhs.shape()[1] != rhs.shape()[0]) {
        throw std::invalid_argument("matrix shapes do not match");
    }

    switch (alg) {
        case algorithm::naive:
            matmul_naive(lhs, rhs, res);
            break;
        case algorithm::transpose:
            matmul_transpose(lhs, rhs, res);
            break;
        case algorithm::divide_and_conquer:
            matmul_divide_and_conquer(lhs, rhs, res);
            break;
        case algorithm::tiled:
            matmul_tiled(lhs, rhs, res);
            break;
    }
}

matrix matmul(const_matrix_ref lhs, const_matrix_ref rhs, algorithm alg) {
    auto nrows = lhs.shape()[0];
    auto ncols = rhs.shape()[1];
    auto res_shape = boost::extents[nrows][ncols];
    matrix res(res_shape);
    matmul(lhs, rhs, res, alg);
    return res;
}
