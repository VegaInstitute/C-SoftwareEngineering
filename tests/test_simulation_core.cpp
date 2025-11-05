// tests/test_simulation_core.cpp
#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "libbacktester/execution_simulator.hpp"
#include "libbacktester/orderbook.hpp"
#include "libbacktester/portfolio.hpp"
#include "libbacktester/simulation_core.hpp"
#include "libbacktester/strategy.hpp"
#include "libbacktester/types.hpp"

using libbacktester::ExecutionSimulator;
using libbacktester::Ms;
using libbacktester::OrderBookL3;
using libbacktester::Portfolio;
using libbacktester::SimulationCore;
using libbacktester::Strategy;

// --------------------------- Test scaffolding -----------------------------

namespace {

std::string WriteCsv(const std::string& filename, const std::string& body) {
  std::ofstream out(filename);
  out << "NO,SECCODE,BUYSELL,TIME,ORDERNO,ACTION,PRICE,VOLUME,TRADENO,TRADEPRICE\n";
  out << body;
  return filename;
}

class TestStrategy final : public Strategy {
 public:
  struct Event {
    char kind;               // 'U' (book update) or 'T' (timer)
    std::int64_t ts_ms;
    std::uint64_t seq;       // for updates; 0 for timer
  };

  void onBookUpdate(Ms ts, std::uint64_t seq) noexcept override {
    events_.push_back({'U', ts.count(), seq});
    update_ts_.push_back(ts.count());
  }
  void onTimer(Ms ts) noexcept override {
    events_.push_back({'T', ts.count(), 0});
    tick_ts_.push_back(ts.count());
  }
  void onFill(const libbacktester::FillEvent&) override {}
  std::vector<libbacktester::OrderIntent> popPendingIntents() override { return {}; }

  std::vector<Event> events_;
  std::vector<std::int64_t> update_ts_;
  std::vector<std::int64_t> tick_ts_;
};

}  // namespace

// ------------------------------- Tests ------------------------------------

// Determinism: identical input → identical event streams.
TEST(SimulationCore, DeterminismOutputsIdentical) {
  const std::string csv_path = WriteCsv("sim_det.csv",
      "1,TKR1,B,100000000000,111,1,100.0,10,,\n"    // 10:00:00.000
      "2,TKR1,S,100001000000,222,1,101.0,10,,\n");  // 10:00:01.000

  // First run
  std::string ev_a;
  {
    TestStrategy strategy;
    OrderBookL3 book("TKR1");
    Portfolio pf;
    ExecutionSimulator exec(&book, &strategy, &pf);
    SimulationCore core(&book, &strategy, &exec, &pf, /*metrics=*/nullptr);

    core.run(csv_path, Ms{1000});

    std::ostringstream os;
    for (const auto& e : strategy.events_) os << e.kind << "," << e.ts_ms << "," << e.seq << "\n";
    ev_a = std::move(os).str();
  }

  // Second run
  std::string ev_b;
  {
    TestStrategy strategy;
    OrderBookL3 book("TKR1");
    Portfolio pf;
    ExecutionSimulator exec(&book, &strategy, &pf);
    SimulationCore core(&book, &strategy, &exec, &pf, /*metrics=*/nullptr);

    core.run(csv_path, Ms{1000});

    std::ostringstream os;
    for (const auto& e : strategy.events_) os << e.kind << "," << e.ts_ms << "," << e.seq << "\n";
    ev_b = std::move(os).str();
  }

  EXPECT_EQ(ev_a, ev_b);
}

// Ordering when event ts == timer ts: all market updates first, then timer at the same ts.
TEST(SimulationCore, Ordering_MarketBeforeTimer_WhenEqualTs) {
  const std::string csv_path = WriteCsv("sim_equal_ts.csv",
      "1,TKR1,B,100001000000,10,1,100.0,10,,\n"   // 10:00:01.000
      "2,TKR1,S,100001000000,20,1,101.0,10,,\n"); // 10:00:01.000

  TestStrategy strategy;
  OrderBookL3 book("TKR1");
  Portfolio pf;
  ExecutionSimulator exec(&book, &strategy, &pf);
  SimulationCore core(&book, &strategy, &exec, &pf, /*metrics=*/nullptr);

  core.run(csv_path, Ms{1000});

  ASSERT_FALSE(strategy.events_.empty());
  const auto ts0 = strategy.events_.front().ts_ms;  // should be 10:00:00.000 (first tick) or 10:00:01.000
  // We need the timestamp where the *two updates* happen:
  // Find first 'U' and use its ts.
  std::int64_t updates_ts = -1;
  for (const auto& e : strategy.events_) {
    if (e.kind == 'U') { updates_ts = e.ts_ms; break; }
  }
  ASSERT_NE(updates_ts, -1);

  int updates_at_ts = 0;
  int timers_at_ts = 0;
  bool seen_timer_after_updates = false;

  for (const auto& e : strategy.events_) {
    if (e.ts_ms != updates_ts) continue;
    if (e.kind == 'U') {
      ++updates_at_ts;
      ASSERT_EQ(timers_at_ts, 0) << "Timer fired before update at equal timestamp";
    } else if (e.kind == 'T') {
      ++timers_at_ts;
      seen_timer_after_updates = true;
    }
  }

  EXPECT_EQ(updates_at_ts, 2);
  EXPECT_EQ(timers_at_ts, 1);
  EXPECT_TRUE(seen_timer_after_updates);
}

// Tick alignment & count: floor to first ts, then stride. Expect ticks at 00, 01, 02, 03.
TEST(SimulationCore, TickCountAndAlignment) {
  const std::string csv_path = WriteCsv("sim_ticks.csv",
      "1,TKR1,B,100000000000,1,1,100.0,10,,\n"     // 10:00:00.000
      "2,TKR1,S,100003000000,2,1,101.0,10,,\n");   // 10:00:03.000

  TestStrategy strategy;
  OrderBookL3 book("TKR1");
  Portfolio pf;
  ExecutionSimulator exec(&book, &strategy, &pf);
  SimulationCore core(&book, &strategy, &exec, &pf, /*metrics=*/nullptr);

  core.run(csv_path, Ms{1000});

  // First record at exactly 10:00:00.000 → ticks at 00,01,02,03 → 4 ticks.
  ASSERT_EQ(strategy.tick_ts_.size(), 4u);
}

// Ticker filtering: lock to the first non-empty SECCODE encountered.
TEST(SimulationCore, TickerFilter_FirstNonEmptySeccode) {
  // Mixed symbols; TKR1 first → only TKR1 rows processed.
  const std::string csv_path = WriteCsv("sim_filter.csv",
      "1,TKR1,B,100000000000,10,1,100.0,10,,\n"      // TKR1
      "2,TKR2,S,100000010000,20,1,101.0,10,,\n"      // TKR2 (ignored)
      "3,TKR1,B,100000020000,30,1,100.5,10,,\n"      // TKR1
      "4,TKR2,S,100000030000,40,1,101.5,10,,\n");    // TKR2 (ignored)

  TestStrategy strategy;
  OrderBookL3 book("TKR1");
  Portfolio pf;
  ExecutionSimulator exec(&book, &strategy, &pf);
  SimulationCore core(&book, &strategy, &exec, &pf, /*metrics=*/nullptr);

  core.run(csv_path, Ms{1000});

  int update_count = 0;
  for (const auto& e : strategy.events_) if (e.kind == 'U') ++update_count;
  EXPECT_EQ(update_count, 2);
}

// TIME parsing variants: HHMMSSZZZ, HHMMSSZZZXXX (µs), and integer milliseconds.
TEST(SimulationCore, TimeParsingVariants) {
  // 10:00:00.000 (9-digit) → 10:00:00.000 (12-digit) → 10:00:01.500 (int ms = 36001500)
  const std::string csv_path = WriteCsv("sim_time_variants.csv",
      "1,TKR1,B,100000000,10,1,100.0,10,,\n"         // 9-digit HHMMSSZZZ → 10:00:00.000
      "2,TKR1,S,100000000123,20,1,101.0,10,,\n"      // 12-digit HHMMSSZZZXXX → 10:00:00.000
      "3,TKR1,B,36001500,30,1,100.5,10,,\n");        // integer ms since midnight → 10:00:01.500

  TestStrategy strategy;
  OrderBookL3 book("TKR1");
  Portfolio pf;
  ExecutionSimulator exec(&book, &strategy, &pf);
  SimulationCore core(&book, &strategy, &exec, &pf, /*metrics=*/nullptr);

  core.run(csv_path, Ms{1000});

  ASSERT_GE(strategy.update_ts_.size(), 3u);
  EXPECT_LE(strategy.update_ts_[0], strategy.update_ts_[1]);
  EXPECT_LE(strategy.update_ts_[1], strategy.update_ts_[2]);
}

// Empty data after header: no events.
TEST(SimulationCore, EmptyAfterHeader_NoOps) {
  const std::string csv_path = WriteCsv("sim_empty.csv", "");

  TestStrategy strategy;
  OrderBookL3 book("TKR1");
  Portfolio pf;
  ExecutionSimulator exec(&book, &strategy, &pf);
  SimulationCore core(&book, &strategy, &exec, &pf, /*metrics=*/nullptr);

  core.run(csv_path, Ms{1000});

  EXPECT_TRUE(strategy.events_.empty());
}
