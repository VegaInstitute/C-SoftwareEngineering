#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "libbacktester/strategies/twap_strategy.hpp"
#include "libbacktester/strategy.hpp"
#include "libbacktester/types.hpp"

using libbacktester::OrderIntent;
using libbacktester::OrderSide;
using libbacktester::TWAPStrategy;
using libbacktester::Ms;

using namespace std::chrono_literals; // enables 1000ms literal

// ------- helpers --------------------------------------------------------------

static std::vector<OrderIntent> TickAndPop(TWAPStrategy& s, Ms ts) {
  s.onTimer(ts);
  return s.popPendingIntents();
}

static std::vector<OrderIntent> FlushAll(TWAPStrategy& s) {
  s.onTimer(Ms{std::numeric_limits<long long>::max()});
  return s.popPendingIntents();
}

static std::int64_t SumQty(const std::vector<OrderIntent>& v) {
  std::int64_t sum = 0;
  for (const auto& oi : v) sum += oi.qty;
  return sum;
}

// ------- tests ---------------------------------------------------------------

TEST(TWAP, SingleSlice_AllAtEnd) {
  TWAPStrategy strat(
      /*strategy_id=*/"tw1",
      /*side=*/OrderSide::kBuy,
      /*total_qty=*/1000,
      /*start_ts=*/1'000ms,
      /*end_ts=*/11'000ms,
      /*slices=*/1);

  // Before end → nothing
  {
    auto out = TickAndPop(strat, 10'999ms);
    EXPECT_TRUE(out.empty());
    EXPECT_EQ(strat.nextDueIndex(), 0u);
    EXPECT_EQ(strat.lastPoppedIndex(), 0u);
  }

  // At end → one intent
  {
    auto out = TickAndPop(strat, 11'000ms);
    ASSERT_EQ(out.size(), 1u);
    const auto& oi = out[0];
    EXPECT_EQ(oi.strategy_id, "tw1");
    EXPECT_EQ(oi.local_id, "tw1-1");
    EXPECT_EQ(oi.side, OrderSide::kBuy);
    EXPECT_FALSE(oi.price.has_value());
    EXPECT_EQ(oi.qty, 1000);
    EXPECT_EQ(oi.submit_ts, 11'000ms);
  }

  // No double emission
  EXPECT_TRUE(strat.popPendingIntents().empty());
}

TEST(TWAP, EqualSplit_NoRemainder_TimingGrid) {
  // span = 5000ms, N=5 → due at start+{1000,2000,3000,4000} and end(=6000)
  TWAPStrategy strat("tw2", OrderSide::kSell, 100, 1'000ms, 6'000ms, 5);
  EXPECT_EQ(strat.totalIntentsMaterialized(), 5u);

  for (int i = 1; i <= 4; ++i) {
    const auto ts = 1'000ms + i * 1'000ms;
    auto out = TickAndPop(strat, ts);
    ASSERT_EQ(out.size(), 1u) << "at step " << i;
    EXPECT_EQ(out[0].qty, 20);
    EXPECT_EQ(out[0].submit_ts, ts);
    EXPECT_EQ(out[0].local_id, "tw2-" + std::to_string(i));
  }

  {
    auto out = TickAndPop(strat, 6'000ms);
    ASSERT_EQ(out.size(), 1u);
    EXPECT_EQ(out[0].qty, 20);
    EXPECT_EQ(out[0].submit_ts, 6'000ms);
    EXPECT_EQ(out[0].local_id, "tw2-5");
  }
}

TEST(TWAP, RemainderPrefix_Distribution) {
  // total=103, N=5 → base=20, rem=3 → {21,21,21,20,20}
  TWAPStrategy strat("tw3", OrderSide::kBuy, 103, 0ms, 5'000ms, 5);
  auto all = FlushAll(strat);
  ASSERT_EQ(all.size(), 5u);

  std::vector<std::int64_t> expected{21, 21, 21, 20, 20};
  for (std::size_t i = 0; i < all.size(); ++i) {
    EXPECT_EQ(all[i].qty, expected[i]) << "slice " << (i + 1);
    EXPECT_EQ(all[i].local_id, "tw3-" + std::to_string(i + 1));
  }
  EXPECT_EQ(SumQty(all), 103);
  EXPECT_EQ(all.back().submit_ts, 5'000ms);
}

TEST(TWAP, ForceAllEqual_LastCollectsRemainder) {
  // total=103, N=5, force_all_equal → first 4=20, last=23
  TWAPStrategy strat("tw4", OrderSide::kSell, 103, 7'000ms, 12'000ms, 5, /*force_all_equal=*/true);
  auto all = FlushAll(strat);
  ASSERT_EQ(all.size(), 5u);

  std::vector<std::int64_t> expected{20, 20, 20, 20, 23};
  for (std::size_t i = 0; i < all.size(); ++i) {
    EXPECT_EQ(all[i].qty, expected[i]) << "slice " << (i + 1);
  }
  EXPECT_EQ(SumQty(all), 103);
  EXPECT_EQ(all.back().submit_ts, 12'000ms);
}

TEST(TWAP, TotalLessThanSlices_SkipZeroQty) {
  // total=3, N=5 → default: first 3 get 1, last 2 get 0 → only 3 intents
  TWAPStrategy strat("tw5", OrderSide::kBuy, 3, 1'000ms, 6'000ms, 5);
  auto all = FlushAll(strat);
  ASSERT_EQ(all.size(), 3u);

  EXPECT_EQ(all[0].local_id, "tw5-1");
  EXPECT_EQ(all[1].local_id, "tw5-2");
  EXPECT_EQ(all[2].local_id, "tw5-3");

  EXPECT_EQ(all[0].qty, 1);
  EXPECT_EQ(all[1].qty, 1);
  EXPECT_EQ(all[2].qty, 1);

  // With span=5000ms, N=5 → due at {2000,3000,4000,5000,6000}; only first 3 exist.
  EXPECT_EQ(all[0].submit_ts, 2'000ms);
  EXPECT_EQ(all[1].submit_ts, 3'000ms);
  EXPECT_EQ(all[2].submit_ts, 4'000ms);
}

TEST(TWAP, SpanLessThanSlices_TiesAreStable) {
  // span=2ms, N=5 → floor(i*2/5) for i=1..4 = {0,0,1,1}; last=end=start+2ms.
  TWAPStrategy strat("tw6", OrderSide::kSell, 10, 100ms, 102ms, 5);

  // At start (100ms) → two intents due (i=1,2) in that order.
  {
    auto out = TickAndPop(strat, 100ms);
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[0].local_id, "tw6-1");
    EXPECT_EQ(out[1].local_id, "tw6-2");
    EXPECT_EQ(out[0].submit_ts, 100ms);
    EXPECT_EQ(out[1].submit_ts, 100ms);
  }

  // At 101ms → next two (i=3,4)
  {
    auto out = TickAndPop(strat, 101ms);
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[0].local_id, "tw6-3");
    EXPECT_EQ(out[1].local_id, "tw6-4");
    EXPECT_EQ(out[0].submit_ts, 101ms);
    EXPECT_EQ(out[1].submit_ts, 101ms);
  }

  // At end=102ms → last slice (i=5)
  {
    auto out = TickAndPop(strat, 102ms);
    ASSERT_EQ(out.size(), 1u);
    EXPECT_EQ(out[0].local_id, "tw6-5");
    EXPECT_EQ(out[0].submit_ts, 102ms);
  }

  EXPECT_TRUE(strat.popPendingIntents().empty());
}

TEST(TWAP, NoDoubleEmission_WhenCalledRepeatedly) {
  TWAPStrategy strat("tw7", OrderSide::kBuy, 50, 0ms, 5'000ms, 5);

  auto first = TickAndPop(strat, 5'000ms);
  ASSERT_EQ(first.size(), 5u);

  auto second = TickAndPop(strat, 5'000ms);
  EXPECT_TRUE(second.empty());

  auto third = strat.popPendingIntents();
  EXPECT_TRUE(third.empty());
}

TEST(TWAP, Determinism_SameInputsSameOutputs) {
  TWAPStrategy a("tw8", OrderSide::kSell, 123, 10'000ms, 20'000ms, 7);
  TWAPStrategy b("tw8", OrderSide::kSell, 123, 10'000ms, 20'000ms, 7);

  std::vector<Ms> times{10'500ms, 12'345ms, 14'285ms, 17'000ms, 20'000ms, 20'000ms};
  std::vector<OrderIntent> out_a, out_b;

  for (auto t : times) {
    auto v = TickAndPop(a, t);
    out_a.insert(out_a.end(), v.begin(), v.end());
  }
  for (auto t : times) {
    auto v = TickAndPop(b, t);
    out_b.insert(out_b.end(), v.begin(), v.end());
  }

  ASSERT_EQ(out_a.size(), out_b.size());
  for (std::size_t i = 0; i < out_a.size(); ++i) {
    EXPECT_EQ(out_a[i].strategy_id, out_b[i].strategy_id);
    EXPECT_EQ(out_a[i].local_id,    out_b[i].local_id);
    EXPECT_EQ(out_a[i].side,        out_b[i].side);
    EXPECT_EQ(out_a[i].qty,         out_b[i].qty);
    EXPECT_EQ(out_a[i].submit_ts,   out_b[i].submit_ts);
  }

  // Flushing remainder is also identical
  auto tail_a = FlushAll(a);
  auto tail_b = FlushAll(b);
  ASSERT_EQ(tail_a.size(), tail_b.size());
  for (std::size_t i = 0; i < tail_a.size(); ++i) {
    EXPECT_EQ(tail_a[i].local_id,  tail_b[i].local_id);
    EXPECT_EQ(tail_a[i].qty,       tail_b[i].qty);
    EXPECT_EQ(tail_a[i].submit_ts, tail_b[i].submit_ts);
  }
}

TEST(TWAP, BookUpdate_DoesNotAffectSchedule) {
  TWAPStrategy strat("tw9", OrderSide::kBuy, 10, 1'000ms, 6'000ms, 5);

  // Book tick shouldn't change indices
  strat.onBookUpdate(1'234ms, /*seq=*/42);
  EXPECT_EQ(strat.nextDueIndex(), 0u);
  EXPECT_EQ(strat.lastPoppedIndex(), 0u);

  // At 3'000ms we should get two intents (due at 2'000 and 3'000).
  auto out = TickAndPop(strat, 3'000ms);
  ASSERT_EQ(out.size(), 2u);
  EXPECT_EQ(out[0].submit_ts, 2'000ms);
  EXPECT_EQ(out[1].submit_ts, 3'000ms);
}

TEST(TWAP, ForceAllEqual_TotalLessThanSlices_OnlyLastExists) {
  // total=3, N=5, force_all_equal → slices 1..4=0, slice 5=3
  TWAPStrategy strat("tw10", OrderSide::kSell, 3, 10'000ms, 10'100ms, 5, /*force_all_equal=*/true);
  auto all = FlushAll(strat);
  ASSERT_EQ(all.size(), 1u);
  EXPECT_EQ(all[0].local_id, "tw10-5");
  EXPECT_EQ(all[0].qty, 3);
  EXPECT_EQ(all[0].submit_ts, 10'100ms);
}

TEST(TWAP, ConstructorValidation) {
  // Invalid strategy id
  EXPECT_THROW(
      TWAPStrategy("", OrderSide::kBuy, 1, 0ms, 1ms, 1), std::invalid_argument);
  // Non-positive qty
  EXPECT_THROW(
      TWAPStrategy("id", OrderSide::kBuy, 0, 0ms, 1ms, 1), std::invalid_argument);
  // Slices == 0
  EXPECT_THROW(
      TWAPStrategy("id", OrderSide::kBuy, 1, 0ms, 1ms, 0), std::invalid_argument);
  // end <= start
  EXPECT_THROW(
      TWAPStrategy("id", OrderSide::kBuy, 1, 1ms, 1ms, 1), std::invalid_argument);
}
