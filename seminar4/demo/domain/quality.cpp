#include "quality.hpp"
// #include "iss_source.hpp"  // DEMO-1

#include <cstddef>

namespace rp {

std::int64_t count_gaps(const std::vector<Bar>& bars, std::int64_t step_ms) {
    std::int64_t gaps = 0;
    for (std::size_t i = 1; i < bars.size(); ++i) {
        const std::int64_t diff = bars[i].ts_ms - bars[i - 1].ts_ms;
        if (diff > step_ms) {
            gaps += diff / step_ms - 1;
        }
    }
    return gaps;
}

}  // namespace rp
