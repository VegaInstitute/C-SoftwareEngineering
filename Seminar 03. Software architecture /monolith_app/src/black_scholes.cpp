#include "options_pricing.h"
#include <cmath>

double norm_cdf(double value)
{
    return 0.5 * erfc(-value * M_SQRT1_2);
}

double CalculatorForEuropeanCallOption::calculate() {
    double d1 = (log(spot / strike) + (interest_rate + volatility * volatility / 2) * maturity) / (volatility * sqrt(maturity));
    double d2 = d1 - volatility * sqrt(maturity);

    return spot * norm_cdf(d1) - strike * exp(-interest_rate * maturity) * norm_cdf(d2);
}