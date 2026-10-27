#include <algorithm>
#include <charconv>
#include <cstdint>
#include <locale>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>

namespace integer_solution {

bool only_digits(std::string_view text) {
    return !text.empty() && std::all_of(text.begin(), text.end(), [](char c) {
        return c >= '0' && c <= '9';
    });
}

std::optional<std::int64_t> parse_price_from_chars(std::string_view text) {
    if (!only_digits(text)) return std::nullopt;
    std::int64_t value = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size() || value <= 0) {
        return std::nullopt;
    }
    return value;
}

std::optional<std::int64_t> parse_price_stream(std::string_view text) {
    if (!only_digits(text)) return std::nullopt;
    std::istringstream input{std::string(text)};
    input.imbue(std::locale::classic());
    std::int64_t value = 0;
    input >> std::noskipws >> value;
    if (!input || input.peek() != std::char_traits<char>::eof() || value <= 0) {
        return std::nullopt;
    }
    return value;
}

std::optional<std::int64_t> bad_parse_prefix(std::string_view text) {
    if (text.empty()) return std::nullopt;
    std::int64_t value = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || value <= 0) return std::nullopt;
    return value;
}

}
