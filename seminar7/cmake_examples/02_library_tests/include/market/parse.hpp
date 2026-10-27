#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace market {
std::optional<std::int64_t> parse_positive_price(std::string_view text);
}
