#include "market/parse.hpp"

#include <iostream>

int main() {
    int checks = 0;
    int failures = 0;
    const auto require = [&](bool condition, const char* message) {
        ++checks;
        if (!condition) {
            std::cerr << "FAIL: " << message << '\n';
            ++failures;
        }
    };
    using market::parse_positive_price;

    require(parse_positive_price("1") == 1, "smallest positive price");
    require(parse_positive_price("12345") == 12345, "ordinary price");
    require(parse_positive_price("00012") == 12, "leading zeroes");
    require(parse_positive_price("9223372036854775807") == INT64_C(9223372036854775807), "int64 maximum");
    require(parse_positive_price(std::string_view{"12345x", 5}) == 12345, "view boundary");
    require(!parse_positive_price(""), "empty input");
    require(!parse_positive_price("0"), "zero");
    require(!parse_positive_price("000"), "zero with leading zeroes");
    require(!parse_positive_price("-1"), "minus sign");
    require(!parse_positive_price("+1"), "plus sign");
    require(!parse_positive_price(" 12"), "leading space");
    require(!parse_positive_price("12 "), "trailing space");
    require(!parse_positive_price("\t12"), "tab");
    require(!parse_positive_price("12\n"), "newline");
    require(!parse_positive_price("12x"), "unconsumed suffix");
    require(!parse_positive_price("12.5"), "fractional price");
    require(!parse_positive_price("1e3"), "exponent notation");
    require(!parse_positive_price("9223372036854775808"), "int64 overflow");
    require(!parse_positive_price("99999999999999999999999999"), "large overflow");
    require(!parse_positive_price("\xB2"), "non-ASCII digit");

    std::cout << checks - failures << '/' << checks << " checks passed\n";
    return failures == 0 ? 0 : 1;
}
