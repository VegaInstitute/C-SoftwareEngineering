#include <stdexcept>
#include "bsm.h"

bsm::BSModelParams::BSModelParams(double volatility) {
    if (volatility < 0) {
        throw std::invalid_argument("Volatility can't be negative");
    }
    this->vola = volatility;
}

double bsm::BSModelParams::total_variance(double dt_years) const {
    if (dt_years < 0) {
        throw std::invalid_argument("Time span can't be negative");
    }
    return this->vola * this->vola * dt_years;
}
