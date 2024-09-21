#include "options_pricing.h"
#include "ui.h"
#include "database.h"
#include <iostream>

int main() {
    double strike_price;
    user_input("Enter the strike price: ").get_data(strike_price);
    
    auto option = EuropeanCallOption(strike_price, 1);
    auto model = ModelBlackScholesMerton(4e-2, 0.1);
    auto calculator = CalculatorForEuropeanCallOption(option, get_spot_price(), model); 

    std::cout << calculator.get_traded_value() << std::endl;
    return 0;
}