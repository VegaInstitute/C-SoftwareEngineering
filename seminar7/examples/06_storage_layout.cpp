#include <cstdint>
#include <optional>
#include <stdexcept>
#include <vector>

namespace storage_demo {

struct Trade {
    std::int64_t time_us;
    std::uint32_t symbol_id;
    std::int64_t price_ticks;
    std::int64_t qty;
};

struct Columns {
    std::vector<std::int64_t> time_us;
    std::vector<std::uint32_t> symbol_id;
    std::vector<std::int64_t> price_ticks;
    std::vector<std::int64_t> qty;
};

std::optional<long double> good_vwap_aos(const std::vector<Trade>& trades) {
    long double amount = 0;
    long double volume = 0;
    for (const auto& trade : trades) {
        if (trade.price_ticks <= 0 || trade.qty <= 0) {
            throw std::invalid_argument("positive price and quantity required");
        }
        amount += static_cast<long double>(trade.price_ticks) * trade.qty;
        volume += trade.qty;
    }
    if (volume == 0) return std::nullopt;
    return amount / volume;
}

std::optional<long double> good_vwap_soa(const Columns& trades) {
    const auto size = trades.price_ticks.size();
    if (trades.qty.size() != size || trades.time_us.size() != size ||
        trades.symbol_id.size() != size) {
        throw std::invalid_argument("column lengths differ");
    }
    long double amount = 0;
    long double volume = 0;
    for (std::size_t i = 0; i < size; ++i) {
        if (trades.price_ticks[i] <= 0 || trades.qty[i] <= 0) {
            throw std::invalid_argument("positive price and quantity required");
        }
        amount += static_cast<long double>(trades.price_ticks[i]) * trades.qty[i];
        volume += trades.qty[i];
    }
    if (volume == 0) return std::nullopt;
    return amount / volume;
}

std::optional<long double> bad_unweighted_mean(const std::vector<Trade>& trades) {
    if (trades.empty()) return std::nullopt;
    long double sum = 0;
    for (const auto& trade : trades) sum += trade.price_ticks;
    return sum / trades.size();
}

std::vector<Trade> sample_aos() {
    return {{0, 1, 100, 1}, {1, 1, 110, 9}};
}

Columns sample_soa() {
    return {{0, 1}, {1, 1}, {100, 110}, {1, 9}};
}

}
