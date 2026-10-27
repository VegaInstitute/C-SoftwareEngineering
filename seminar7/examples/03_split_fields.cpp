#include <array>
#include <optional>
#include <stdexcept>
#include <string_view>

namespace split_exercise {

using Fields = std::array<std::string_view, 5>;

std::optional<Fields> split_fields(std::string_view) {
    throw std::logic_error("TODO: split exactly five nonempty semicolon fields");
}

struct Question {
    std::string_view line;
    bool accepted_by_split;
};

constexpr std::array<Question, 9> questions{{
    {"1761566400000000;SBER;28543;10;B", true},
    {"0;A;1;1;S", true},
    {"0;A;1x;1;S", true},
    {"", false},
    {"0;A;1;1", false},
    {"0;;1;1;B", false},
    {"0;A;1;1;", false},
    {"0;A;1;1;B;extra", false},
    {"0;A;1;1;B;", false}
}};

}
