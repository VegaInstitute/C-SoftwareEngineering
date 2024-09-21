#ifndef PRODUCTS_H
#define PRODUCTS_H

#include <string>
#include <stdexcept>

class EuropeanProduct {
private:
    std::string name;
    double maturity;

public:
    EuropeanProduct(std::string name, double maturity) : name(name), maturity(maturity) {
        if (maturity <= 0) {
            throw std::invalid_argument("T must be positive");
        }
    }
    std::string get_name() const { return name; }
    double get_maturity() const { return maturity; }
};

class EuropeanCallOption : public EuropeanProduct {
private:
    double strike;

public:
    EuropeanCallOption(double strike, double maturity) : EuropeanProduct("European Call Option", maturity), strike(strike) {
        if (strike <= 0) {
            throw std::invalid_argument("K must be positive");
        }
    }
    double get_strike() const { return strike; }
    
    double get_payoff(double S) const {return std::max(S - strike, 0.0); }
};

#endif // PRODUCTS_H