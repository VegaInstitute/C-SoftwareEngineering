#include "header.h"
#include <iostream>

double get_spot_price() {
    return 100;
}

int main() {
    double strike_price;

    std::cout << "Enter the strike price: ";
    std::cin >> strike_price;
    if (strike_price <= 0) {
        std::cerr << "Strike price must be positive." << std::endl;
        return 1;
    }

    double traded_value = calculate_call_price(get_spot_price(), strike_price, 1, 4e-2, 0.1);
    std::cout << traded_value << std::endl;
    return 0;
}