#include <iostream>
#include <cstdlib>
#include "bsm.h"


int main() {

    double vol, dt;

    std::cout << "Volatility: ";
    std::cin >> vol;
    std::cout << "Time span in years: ";
    std::cin >> dt;
    std::cout << "Total variance is " << bsm::BSModelParams(vol).total_variance(dt) << '\n';

    return EXIT_SUCCESS;
}
