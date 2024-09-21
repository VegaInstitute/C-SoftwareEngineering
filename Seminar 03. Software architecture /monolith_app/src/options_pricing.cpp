#include "options_pricing.h"
#include <iostream>

double get_spot_price() {
    // In a real application, this would query a data source like a database
    return 100;
}

int main() {
    double strike_price;

    std::cout << "Enter the strike price: ";
    std::cin >> strike_price;
    
    auto calculator = CalculatorForEuropeanCallOption(get_spot_price(), strike_price, 1, 4e-2, 0.1); 

    std::cout << calculator.get_traded_value() << std::endl;
    return 0;
}
