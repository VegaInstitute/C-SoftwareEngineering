#include "market/parse.hpp"

#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: price_app <positive-price-in-ticks>\n";
        return 2;
    }
    const auto price = market::parse_positive_price(argv[1]);
    if (!price) {
        std::cerr << "Invalid price\n";
        return 1;
    }
    std::cout << "price = " << *price << " ticks\n";
}
