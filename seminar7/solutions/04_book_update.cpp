#include <algorithm>
#include <cstdint>
#include <map>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace book_solution {

using Number = std::int64_t;
using Level = std::pair<Number, Number>;

void validate_set(Number price, Number qty) {
    if (price <= 0 || qty < 0) throw std::invalid_argument("invalid SET");
}

class MapBook {
    std::map<Number, Number> levels_;
public:
    void set(Number price, Number qty) {
        validate_set(price, qty);
        if (qty == 0) levels_.erase(price);
        else levels_[price] = qty;
    }

    Number quantity(Number price) const {
        const auto position = levels_.find(price);
        return position == levels_.end() ? 0 : position->second;
    }

    std::optional<Level> best_bid() const {
        if (levels_.empty()) return std::nullopt;
        return *levels_.rbegin();
    }
};

class SortedVectorBook {
    std::vector<Level> levels_;
public:
    void set(Number price, Number qty) {
        validate_set(price, qty);
        const auto position = std::lower_bound(levels_.begin(), levels_.end(), price,
            [](const Level& level, Number target) { return level.first < target; });
        if (position != levels_.end() && position->first == price) {
            if (qty == 0) levels_.erase(position);
            else position->second = qty;
        } else if (qty != 0) {
            levels_.insert(position, {price, qty});
        }
    }

    Number quantity(Number price) const {
        const auto position = std::lower_bound(levels_.begin(), levels_.end(), price,
            [](const Level& level, Number target) { return level.first < target; });
        return position != levels_.end() && position->first == price ? position->second : 0;
    }

    std::optional<Level> best_bid() const {
        if (levels_.empty()) return std::nullopt;
        return levels_.back();
    }
};

}
