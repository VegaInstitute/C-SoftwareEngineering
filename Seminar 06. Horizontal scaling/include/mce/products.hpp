#ifndef PRODUCTS_HPP
#define PRODUCTS_HPP

#include <algorithm>

template <typename T>
class EuropeanOption {
public:
    virtual double payoff(double spot) const = 0;
    virtual ~EuropeanOption() = default;
};

// Curiously recursive template pattern
class EuropeanCallOption : public EuropeanOption<EuropeanCallOption> {
private:
    double strike;
    double maturity;

public:
    EuropeanCallOption(double strike, double maturity) : strike(strike), maturity(maturity) {}

    double payoff(double spot) const override {
        return std::max(spot - strike, 0.);
    };

    double get_maturity() const { return maturity; };
};


#endif // PRODUCTS_HPP
