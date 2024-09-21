#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <stdexcept>

class Model {
private:
    std::string name;

public:
    Model(std::string name) : name(name) {}
    std::string get_name() const { return name; }
};

class ModelBlackScholesMerton : public Model {
private:
    std::string name;
    double r;
    double sigma;

public:
    ModelBlackScholesMerton(double r, double sigma) : Model(std::string("Black-Scholes-Merton")), r(r), sigma(sigma) {}

    double get_interest_rate() const { return r; }
    double get_volatility() const { return sigma; }
};

#endif // MODELS_H