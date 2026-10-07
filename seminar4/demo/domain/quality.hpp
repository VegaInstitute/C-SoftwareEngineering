#pragma once

#include <cstdint>
#include <vector>

#include "bar_source.hpp"

namespace rp {

std::int64_t count_gaps(const std::vector<Bar>& bars, std::int64_t step_ms);

}  // namespace rp
