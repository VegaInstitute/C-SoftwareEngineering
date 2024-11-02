#include <unistd.h>

#include <array>
#include <cmath>
#include <functional>
#include <iostream>
#include <random>
#include <type_traits>

#include "matmul.h"

std::ostream& operator<<(std::ostream& os, const matrix& m) {
    for (const auto& row : m) {
        for (const auto& elem : row) {
            os << elem << '\t';
        }
        os << '\n';
    }
    return os;
}

int main() {
    double data[] = {1, 2, 3, 4};
    matrix_ref m(data, boost::extents[2][2]);
    auto view = m[boost::indices[range(0, 7)][range(0, 5)]];
    std::cout << m << '\n';
    std::cout << view << '\n';
    view[3][2] = 10.0;
    std::cout << view << '\n';

    return 0;
}
