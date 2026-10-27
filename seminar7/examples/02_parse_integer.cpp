#include <array>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string_view>

namespace integer_exercise {

std::optional<std::int64_t> parse_price(std::string_view) {
    throw std::logic_error("TODO: parse the whole digits-only positive int64 field");
}

struct Question {
    std::string_view text;
    bool accepted;
};

constexpr std::array<Question, 12> questions{{
    {"28543", true},
    {"00042", true},
    {"9223372036854775807", true},
    {"", false},
    {"0", false},
    {"-1", false},
    {"+1", false},
    {" 1", false},
    {"1 ", false},
    {"28543x", false},
    {"1.5", false},
    {"9223372036854775808", false}
}};

}
