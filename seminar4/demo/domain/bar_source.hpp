#pragma once

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>

namespace rp {

struct Bar {
    std::int64_t ts_ms = 0;
    std::int64_t open = 0, high = 0, low = 0, close = 0;
    std::int64_t volume = 0;
    std::string source;
};

struct SourceError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

class IBarSource {
public:
    virtual ~IBarSource() = default;
    virtual std::optional<Bar> next() = 0;
    virtual std::string name() const = 0;
};

}  // namespace rp
