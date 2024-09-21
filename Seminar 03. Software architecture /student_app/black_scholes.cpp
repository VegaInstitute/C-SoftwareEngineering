#include "header.h"
#include <cmath>

double norm_cdf(double value)
{
    return 0.5 * erfc(-value * M_SQRT1_2);
}

double calculate_call_price(double S, double K, double T, double r, double sigma) {
    double d1 = (log(S / K) + (r + sigma * sigma / 2) * T) / (sigma * sqrt(T));
    double d2 = d1 - sigma * sqrt(T);

    return S * norm_cdf(d1) - K * exp(-r * T) * norm_cdf(d2);
}