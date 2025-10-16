#include <gtest/gtest.h>

#include "libbacktester/portfolio.hpp"
#include "libbacktester/types.hpp"

using libbacktester::FillEvent;
using libbacktester::Ms;
using libbacktester::OrderSide;
using libbacktester::Portfolio;

namespace {
FillEvent F(const char* sid, OrderSide side, int64_t qty, double px, int64_t ts_ms) {
  FillEvent f{};
  f.strategy_id = sid;
  f.side = side;
  f.qty = qty;
  f.price = px;
  f.ts = Ms{ts_ms};
  return f;
}
}  // namespace

// Long lifecycle: open, add, partial close, full close.
TEST(PortfolioQA, LongLifecycleAccounting) {
  Portfolio pf;
  pf.onFill(F("s", OrderSide::kBuy, 10, 100.0, 0));   // open
  pf.onFill(F("s", OrderSide::kBuy, 10, 110.0, 1));   // add
  EXPECT_EQ(pf.position(), 20);
  EXPECT_DOUBLE_EQ(pf.avgPrice(), (10 * 100.0 + 10 * 110.0) / 20.0);

  pf.onFill(F("s", OrderSide::kSell, 5, 120.0, 2));   // partial close
  EXPECT_EQ(pf.position(), 15);
  EXPECT_DOUBLE_EQ(pf.realizedPnl(), 5 * (120.0 - ((100.0 + 110.0) / 2.0)));

  pf.onFill(F("s", OrderSide::kSell, 15, 95.0, 3));   // full close
  EXPECT_EQ(pf.position(), 0);
  EXPECT_DOUBLE_EQ(pf.avgPrice(), 0.0);
}

// Short lifecycle: open, add, partial cover, full cover.
TEST(PortfolioQA, ShortLifecycleAccounting) {
  Portfolio pf;
  pf.onFill(F("s", OrderSide::kSell, 8, 50.0, 0));    // open short
  pf.onFill(F("s", OrderSide::kSell, 2, 55.0, 1));    // add
  EXPECT_EQ(pf.position(), -10);
  EXPECT_DOUBLE_EQ(pf.avgPrice(), (8 * 50.0 + 2 * 55.0) / 10.0);

  pf.onFill(F("s", OrderSide::kBuy, 4, 48.0, 2));     // partial cover
  EXPECT_EQ(pf.position(), -6);
  EXPECT_GT(pf.realizedPnl(), 0.0);

  pf.onFill(F("s", OrderSide::kBuy, 6, 60.0, 3));     // full cover
  EXPECT_EQ(pf.position(), 0);
  EXPECT_DOUBLE_EQ(pf.avgPrice(), 0.0);
}

// Flip across zero: basis resets to new side entry price.
TEST(PortfolioQA, CrossZeroFlipsResetAverage) {
  // short -> long
  {
    Portfolio pf;
    pf.onFill(F("s", OrderSide::kSell, 5, 150.0, 0));
    pf.onFill(F("s", OrderSide::kBuy, 8, 140.0, 1));
    EXPECT_EQ(pf.position(), 3);
    EXPECT_DOUBLE_EQ(pf.avgPrice(), 140.0);
  }
  // long -> short
  {
    Portfolio pf;
    pf.onFill(F("s", OrderSide::kBuy, 5, 100.0, 0));
    pf.onFill(F("s", OrderSide::kSell, 8, 110.0, 1));
    EXPECT_EQ(pf.position(), -3);
    EXPECT_DOUBLE_EQ(pf.avgPrice(), 110.0);
  }
}

// Mark-to-market invariants: equity = cash + pos * mid; mid updates only if both sides > 0.
TEST(PortfolioQA, MarkToMarketAndEquityInvariants) {
  Portfolio pf;
  pf.onFill(F("s", OrderSide::kBuy, 10, 100.0, 0));

  pf.markToMarket(99.0, 101.0);  // mid = 100
  EXPECT_DOUBLE_EQ(pf.equity(), -1000.0 + 10 * 100.0);

  pf.markToMarket(98.0, 100.0);  // mid = 99
  EXPECT_DOUBLE_EQ(pf.equity(), -1000.0 + 10 * 99.0);

  const double eq_before = pf.equity();
  pf.markToMarket(0.0, 200.0);   // invalid mid -> unchanged
  EXPECT_DOUBLE_EQ(pf.equity(), eq_before);
}

// Zero-qty fill is benign (no state change).
TEST(PortfolioQA, ZeroQtyFillIsBenign) {
  Portfolio pf;
  pf.onFill(F("s", OrderSide::kBuy, 0, 123.45, 0));
  EXPECT_EQ(pf.position(), 0);
  EXPECT_DOUBLE_EQ(pf.cash(), 0.0);
  EXPECT_DOUBLE_EQ(pf.realizedPnl(), 0.0);
}
