#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace split_solution {

using Fields = std::array<std::string_view, 5>;

std::optional<Fields> split_fields_views(std::string_view line) {
    Fields fields{};
    std::size_t start = 0;
    for (std::size_t i = 0; i < fields.size(); ++i) {
        const auto separator = line.find(';', start);
        const bool last = i + 1 == fields.size();
        if ((!last && separator == std::string_view::npos) ||
            (last && separator != std::string_view::npos)) {
            return std::nullopt;
        }
        const auto end = separator == std::string_view::npos ? line.size() : separator;
        if (end == start) return std::nullopt;
        fields[i] = line.substr(start, end - start);
        if (!last) start = end + 1;
    }
    return fields;
}

std::optional<std::vector<std::string>> split_fields_owning(std::string_view line) {
    const auto views = split_fields_views(line);
    if (!views) return std::nullopt;
    std::vector<std::string> fields;
    fields.reserve(views->size());
    for (const auto field : *views) fields.emplace_back(field);
    return fields;
}

}
