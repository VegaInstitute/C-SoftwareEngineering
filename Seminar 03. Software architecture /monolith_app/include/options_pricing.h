#ifndef OPTIONS_PRICING_H
#define OPTIONS_PRICING_H

#include <string>
#include <stdexcept>


class CalculatorForEuropeanCallOption {
private:
    double spot;
    double strike;
    double maturity;
    double interest_rate;
    double volatility;

    double traded_value;

public:
    CalculatorForEuropeanCallOption(double spot, double strike, double maturity, double interest_rate, double volatility) 
        : spot(spot), strike(strike), maturity(maturity), interest_rate(interest_rate), volatility(volatility) {
        if (spot <= 0) {
            throw std::invalid_argument("S must be positive");
        }
        if (strike <= 0) {
            throw std::invalid_argument("K must be positive");
        }
        if (maturity <= 0) {
            throw std::invalid_argument("T must be positive");
        }
        if (volatility <= 0) {
            throw std::invalid_argument("sigma must be positive");
        }
        
        traded_value = calculate();
    }

    double calculate();
    double get_traded_value() const { return traded_value; }
};

#endif // OPTIONS_PRICING_H
