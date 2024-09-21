#include "options_pricing.h"
#include <cmath>

double norm_cdf(double value)
{
    return 0.5 * erfc(-value * M_SQRT1_2);
}

double CalculatorForEuropeanCallOption::calculate(double spot) {
    double T = product.get_maturity();
    double K = product.get_strike();
    double r = model.get_interest_rate();
    double sigma = model.get_volatility();

    double d1 = (log(spot / K) + (r + sigma * sigma / 2) * T) / (sigma * sqrt(T));
    double d2 = d1 - sigma * sqrt(T);

    return spot * norm_cdf(d1) - K * exp(-r * T) * norm_cdf(d2);
}