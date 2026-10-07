#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "bar_source.hpp"

namespace rp {

class IssSource final : public IBarSource {
public:
    explicit IssSource(const std::filesystem::path& csv,
                       std::optional<std::size_t> fail_after = std::nullopt);

    std::optional<Bar> next() override;
    std::string name() const override { return "iss"; }

private:
    std::vector<Bar> bars_;
    std::size_t pos_ = 0;
    std::optional<std::size_t> fail_after_;
};

}  // namespace rp
