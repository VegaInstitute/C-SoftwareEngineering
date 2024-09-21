#ifndef OPTIONS_PRICING_H
#define OPTIONS_PRICING_H

#include <stdexcept>
#include "models.h"
#include "products.h"


class CalculatorForEuropeanCallOption {
private:
    EuropeanCallOption product;
    ModelBlackScholesMerton model;

    double traded_value;

public:
    CalculatorForEuropeanCallOption(EuropeanCallOption product, double spot, ModelBlackScholesMerton model) : product(product), model(model) {
        if (spot <= 0) {
            throw std::invalid_argument("S must be positive");
        }
        traded_value = calculate(spot);
    }

    double calculate(double spot);
    double get_traded_value() const { return traded_value; }
};

#endif // OPTIONS_PRICING_H
