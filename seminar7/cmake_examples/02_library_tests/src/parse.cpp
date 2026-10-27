#include "market/parse.hpp"

#include <charconv>
#include <system_error>

namespace market {
std::optional<std::int64_t> parse_positive_price(std::string_view text) {
    if (text.empty()) {
        return std::nullopt;
    }

    for (const char ch : text) {
        if (ch < '0' || ch > '9') {
            return std::nullopt;
        }
    }
    
	std::int64_t value = 0;
    const char* begin = text.data();
    const char* end = begin + text.size();
    const auto result = std::from_chars(begin, end, value);
    
	if (result.ec != std::errc{} || result.ptr != end || value <= 0) {
        return std::nullopt;
    }
    
	return value;
}
}
