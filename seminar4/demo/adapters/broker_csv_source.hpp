#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "bar_source.hpp"

namespace rp {

// <TICKER>,<PER>,<DATE>,<TIME>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<VOL>
class BrokerCsvSource final : public IBarSource {
public:
    explicit BrokerCsvSource(const std::filesystem::path& csv);

    std::optional<Bar> next() override;
    std::string name() const override { return "broker"; }

private:
    std::vector<Bar> bars_;
    std::size_t pos_ = 0;
};

}  // namespace rp
