#include "market/parse.hpp"

namespace market {
std::optional<std::int64_t> parse_positive_price(std::string_view) {
    return std::nullopt;
}
}
