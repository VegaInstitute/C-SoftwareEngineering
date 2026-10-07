#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "csv_util.hpp"
#include "failover_source.hpp"
#include "quality.hpp"

namespace {

int failures = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            ++failures;                                                          \
            std::cerr << __FILE__ << ':' << __LINE__ << ": CHECK failed: " #cond "\n"; \
        }                                                                        \
    } while (false)

constexpr std::int64_t kMinute = 60'000;

class FakeSource final : public rp::IBarSource {
public:
    FakeSource(std::string name, std::vector<int> minutes,
               std::optional<std::size_t> throw_after = std::nullopt)
        : name_(std::move(name)), minutes_(std::move(minutes)), throw_after_(throw_after) {}

    std::optional<rp::Bar> next() override {
        if (throw_after_ && pos_ >= *throw_after_) {
            throw rp::SourceError("fake outage");
        }
        if (pos_ == minutes_.size()) {
            return std::nullopt;
        }
        rp::Bar bar;
        bar.ts_ms = minutes_[pos_++] * kMinute;
        bar.source = name_;
        return bar;
    }
    std::string name() const override { return name_; }

private:
    std::string name_;
    std::vector<int> minutes_;
    std::optional<std::size_t> throw_after_;
    std::size_t pos_ = 0;
};

std::vector<rp::Bar> drain(rp::IBarSource& src) {
    std::vector<rp::Bar> out;
    while (auto bar = src.next()) {
        out.push_back(std::move(*bar));
    }
    return out;
}

void primary_ok_backup_unused() {
    rp::FailoverSource src(std::make_unique<FakeSource>("iss", std::vector{0, 1, 2}),
                           std::make_unique<FakeSource>("broker", std::vector{0, 1, 2}));
    const auto bars = drain(src);
    CHECK(bars.size() == 3);
    CHECK(!src.switched());
    for (const auto& b : bars) CHECK(b.source == "iss");
}

void switches_without_duplicates() {
    rp::FailoverSource src(std::make_unique<FakeSource>("iss", std::vector{0, 1, 2, 3, 4}, 3),
                           std::make_unique<FakeSource>("broker", std::vector{0, 1, 2, 3, 5}));
    const auto bars = drain(src);
    CHECK(src.switched());
    CHECK(bars.size() == 5);
    for (std::size_t i = 1; i < bars.size(); ++i) CHECK(bars[i].ts_ms > bars[i - 1].ts_ms);
    CHECK(bars[2].source == "iss");
    CHECK(bars[3].source == "broker");
    CHECK(rp::count_gaps(bars, kMinute) == 1);
}

void primary_down_from_start() {
    rp::FailoverSource src(std::make_unique<FakeSource>("iss", std::vector{0, 1}, 0),
                           std::make_unique<FakeSource>("broker", std::vector{0, 1}));
    const auto bars = drain(src);
    CHECK(bars.size() == 2);
    for (const auto& b : bars) CHECK(b.source == "broker");
}

void prices_are_exact_ticks() {
    CHECK(rp::csv::price_to_ticks("285.43") == 28543);
    CHECK(rp::csv::price_to_ticks("285.4") == 28540);
    CHECK(rp::csv::price_to_ticks("285") == 28500);
}

}  // namespace

int main() {
    primary_ok_backup_unused();
    switches_without_duplicates();
    primary_down_from_start();
    prices_are_exact_ticks();
    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "failover_test: all checks passed\n";
    return 0;
}
