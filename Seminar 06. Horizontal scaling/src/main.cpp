#include <mce/generic_monte_carlo.hpp>
#include <mce/products.hpp>
#include <iostream>

int main() {
    EuropeanCallOption opt(100, 1);
    MonteCarloPricer<EuropeanCallOption> pricer (1000000, 10, 110, 0.04, 0.1, opt);

    std::cout << pricer.get_result().price << std::endl;

    return 0;
}
