#include <algorithm>
#include <array>
#include <string>
#include <string_view>

namespace lifetime_demo {

std::string_view bad_dangling_symbol() {
    std::string line = "1761566400000000;SBER;28543;10;B";
    return std::string_view(line).substr(17, 4);
}

std::string_view bad_reused_buffer(std::array<char, 4>& buffer) {
    const std::string_view saved(buffer.data(), buffer.size());
    std::copy_n("GAZP", buffer.size(), buffer.data());
    return saved;
}

std::string bad_data_as_c_string(std::string_view field) {
    return std::string(field.data());
}

std::string safe_owned_symbol() {
    std::string line = "1761566400000000;SBER;28543;10;B";
    return line.substr(17, 4);
}

std::string_view safe_borrowed_symbol(const std::string& line) {
    return std::string_view(line).substr(17, 4);
}

std::string safe_length_aware_copy(std::string_view field) {
    return std::string(field);
}

std::string safe_copy_before_reuse(std::array<char, 4>& buffer) {
    const std::string saved(buffer.data(), buffer.size());
    std::copy_n("GAZP", buffer.size(), buffer.data());
    return saved;
}

}
