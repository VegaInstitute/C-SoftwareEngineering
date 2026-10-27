#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

constexpr std::int64_t min_price = 100000;
constexpr std::size_t tick_span = 2048;
constexpr std::size_t default_operations = 100000;
constexpr std::size_t repetitions = 7;
using Level = std::pair<std::int64_t, std::int64_t>;
volatile std::uint64_t observable_checksum = 0;

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

class MapBook {
    std::map<std::int64_t, std::int64_t> levels_;
public:
    void update(std::int64_t price, std::int64_t qty) {
        if (qty == 0) {
            levels_.erase(price);
        } else {
            levels_[price] = qty;
        }
    }
    std::optional<Level> best_bid() const {
        if (levels_.empty()) {
            return std::nullopt;
        }
        return *levels_.rbegin();
    }
    std::vector<Level> snapshot() const { return {levels_.begin(), levels_.end()}; }
};

class VectorBook {
    std::vector<Level> levels_;
public:
    VectorBook() { levels_.reserve(tick_span); }
    void update(std::int64_t price, std::int64_t qty) {
        const auto position = std::lower_bound(levels_.begin(), levels_.end(), price,
            [](const Level& level, std::int64_t value) { return level.first < value; });
        if (position != levels_.end() && position->first == price) {
            if (qty == 0) {
                levels_.erase(position);
            } else {
                position->second = qty;
            }
        } else if (qty != 0) {
            levels_.insert(position, {price, qty});
        }
    }
    std::optional<Level> best_bid() const {
        if (levels_.empty()) {
            return std::nullopt;
        }
        return levels_.back();
    }
    std::vector<Level> snapshot() const { return levels_; }
};

class DenseBook {
    std::vector<std::int64_t> quantities_ = std::vector<std::int64_t>(tick_span, 0);
public:
    void update(std::int64_t price, std::int64_t qty) {
        quantities_[static_cast<std::size_t>(price - min_price)] = qty;
    }
    std::optional<Level> best_bid() const {
        for (std::size_t i = quantities_.size(); i != 0; --i) {
            if (quantities_[i - 1] != 0) {
                return Level{min_price + static_cast<std::int64_t>(i - 1), quantities_[i - 1]};
            }
        }
        return std::nullopt;
    }
    std::vector<Level> snapshot() const {
        std::vector<Level> result;
        result.reserve(tick_span);
        for (std::size_t i = 0; i < quantities_.size(); ++i) {
            if (quantities_[i] != 0) {
                result.emplace_back(min_price + static_cast<std::int64_t>(i), quantities_[i]);
            }
        }
        return result;
    }
};

std::uint64_t next_random(std::uint64_t& state) {
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return state;
}

std::vector<Level> generate_updates(std::size_t count, std::size_t active_span) {
    std::uint64_t state = 0xA0761D6478BD642FULL;
    std::vector<Level> updates;
    updates.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        const auto price = min_price + static_cast<std::int64_t>(next_random(state) % active_span);
        const auto value = next_random(state);
        const auto qty = value % 4 == 0 ? 0 : 1 + static_cast<std::int64_t>(value % 200);
        updates.emplace_back(price, qty);
    }
    return updates;
}

void check_each_step() {
    MapBook tree;
    VectorBook sorted;
    DenseBook dense;
    require(!tree.best_bid() && !sorted.best_bid() && !dense.best_bid(), "empty best bid");
    const std::array<Level, 9> updates = {{
        {min_price, 5}, {min_price + 2, 10}, {min_price + 1, 6},
        {min_price + 2, 7}, {min_price + 2, 0}, {min_price, 0},
        {min_price + 1, 0}, {min_price + 1, 0}, {min_price + tick_span - 1, 4}
    }};
    const std::array<std::optional<Level>, 9> expected = {{
        Level{min_price, 5}, Level{min_price + 2, 10}, Level{min_price + 2, 10},
        Level{min_price + 2, 7}, Level{min_price + 1, 6}, Level{min_price + 1, 6},
        std::nullopt, std::nullopt, Level{min_price + tick_span - 1, 4}
    }};
    for (std::size_t i = 0; i < updates.size(); ++i) {
        const auto [price, qty] = updates[i];
        tree.update(price, qty);
        sorted.update(price, qty);
        dense.update(price, qty);
        require(tree.best_bid() == expected[i], "map best bid differs from explicit example");
        require(sorted.best_bid() == expected[i], "vector best bid differs from explicit example");
        require(dense.best_bid() == expected[i], "dense best bid differs from explicit example");
        require(tree.snapshot() == sorted.snapshot() && tree.snapshot() == dense.snapshot(),
                "example state mismatch");
    }
    for (const auto& [price, qty] : generate_updates(256, tick_span)) {
        tree.update(price, qty);
        sorted.update(price, qty);
        dense.update(price, qty);
        require(tree.best_bid() == sorted.best_bid() && tree.best_bid() == dense.best_bid(),
                "small-stream best bid mismatch");
        require(tree.snapshot() == sorted.snapshot() && tree.snapshot() == dense.snapshot(),
                "small-stream state mismatch");
    }
    std::cout << "checks: OK (9 explicit + 256 deterministic updates, state and best after every step)\n";
}

std::uint64_t level_checksum(const Level& level) {
    return static_cast<std::uint64_t>(level.first) * 1315423911ULL +
           static_cast<std::uint64_t>(level.second) * 2654435761ULL;
}

std::uint64_t state_checksum(const std::vector<Level>& levels) {
    std::uint64_t result = 0;
    for (const auto& level : levels) {
        result += level_checksum(level);
    }
    return result;
}

std::uint64_t best_checksum(const std::optional<Level>& best) {
    return best ? level_checksum(*best) : 0;
}

struct Expected {
    std::vector<Level> state;
    std::uint64_t best_sum;
};

template <class Book>
Expected run_for_check(const std::vector<Level>& updates) {
    Book book;
    std::uint64_t best_sum = 0;
    for (const auto& [price, qty] : updates) {
        book.update(price, qty);
        best_sum += best_checksum(book.best_bid());
    }
    return {book.snapshot(), best_sum};
}

Expected check_full_stream(const std::vector<Level>& updates) {
    for (const auto& [price, qty] : updates) {
        require(price >= min_price && price < min_price + static_cast<std::int64_t>(tick_span) && qty >= 0,
                "workload violates prevalidated bounded book contract");
    }
    const auto reference = run_for_check<MapBook>(updates);
    const auto sorted = run_for_check<VectorBook>(updates);
    const auto dense = run_for_check<DenseBook>(updates);
    require(reference.state == sorted.state && reference.state == dense.state,
            "full-stream final state mismatch");
    require(reference.best_sum == sorted.best_sum && reference.best_sum == dense.best_sum,
            "full-stream best checksum mismatch");
    std::cout << "full_stream: OK final_levels=" << reference.state.size()
              << " state_checksum=" << state_checksum(reference.state)
              << " best_checksum=" << reference.best_sum << '\n';
    return reference;
}

template <class Book, bool ReadBest>
double run_once(const std::vector<Level>& updates, const Expected& expected) {
    Book book;
    std::uint64_t best_sum = 0;
    const auto start = std::chrono::steady_clock::now();
    for (const auto& [price, qty] : updates) {
        book.update(price, qty);
        if constexpr (ReadBest) {
            best_sum += best_checksum(book.best_bid());
        }
    }
    const auto stop = std::chrono::steady_clock::now();
    const auto final_state = book.snapshot();
    require(final_state == expected.state, "timed final state mismatch");
    if constexpr (ReadBest) {
        require(best_sum == expected.best_sum, "timed best checksum mismatch");
    }
    observable_checksum = state_checksum(final_state) + best_sum;
    return std::chrono::duration<double, std::nano>(stop - start).count() / updates.size();
}

template <class Book, bool ReadBest>
void benchmark(const char* name, const std::vector<Level>& updates, const Expected& expected) {
    run_once<Book, ReadBest>(updates, expected);
    std::array<double, repetitions> samples{};
    for (auto& sample : samples) {
        sample = run_once<Book, ReadBest>(updates, expected);
    }
    std::sort(samples.begin(), samples.end());
    const auto median = samples[repetitions / 2];
    std::cout << name << " " << (ReadBest ? "update+best" : "update_only")
              << ": median_ns/op=" << median << " ops/s=" << 1e9 / median
              << " checksum=" << observable_checksum << '\n';
}

}

int main(int argc, char** argv) {
    try {
        bool check_only = false;
        std::size_t count = default_operations;
        std::size_t active_span = tick_span;
        for (int i = 1; i < argc; ++i) {
            const std::string_view argument(argv[i]);
            if (argument == "--check") {
                check_only = true;
            } else if (argument == "--quick") {
                count = 10000;
            } else if (argument == "--sparse") {
                active_span = 64;
            } else if (argument == "--help") {
                std::cout << "book_bench [--check] [--quick] [--sparse]\n";
                return 0;
            } else {
                throw std::runtime_error("unknown argument: " + std::string(argument));
            }
        }
        check_each_step();
        const auto updates = generate_updates(count, active_span);
        const auto expected = check_full_stream(updates);
        if (check_only) {
            return 0;
        }
        std::cout << std::fixed << std::setprecision(2) << "operations=" << count
                  << " tick_span=" << tick_span << " active_span=" << active_span
                  << " warmup=1 repeats=" << repetitions << '\n';
        benchmark<MapBook, false>("map", updates, expected);
        benchmark<VectorBook, false>("sorted_vector", updates, expected);
        benchmark<DenseBook, false>("dense_linear_best", updates, expected);
        benchmark<MapBook, true>("map", updates, expected);
        benchmark<VectorBook, true>("sorted_vector", updates, expected);
        benchmark<DenseBook, true>("dense_linear_best", updates, expected);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
