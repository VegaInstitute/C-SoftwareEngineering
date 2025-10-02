#include "libbacktester/orderbook.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <vector>

// Small helper to dump a CSV to disk.
static std::string WriteCsv(const std::string& fname, const std::string& data) {
  std::ofstream f(fname, std::ios::trunc);
  f << data;
  f.close();
  return fname;
}

// ----- Basic add / best-of-book -----

TEST(OrderBookL3Test, AddAndBest) {
  const std::string csv =
      "NO,SECCODE,BUYSELL,TIME,ORDERNO,ACTION,PRICE,VOLUME,TRADENO,TRADEPRICE\n"
      "1,FOO,B,1000,1,1,10.00,100,,\n"
      "2,FOO,S,1000,2,1,11.00,200,,\n";

  backtester::OrderBookL3 ob("FOO");
  ob.LoadCsv(WriteCsv("ob_add_best.csv", csv));

  auto bb = ob.bestBid();
  ASSERT_TRUE(bb.has_value());
  EXPECT_DOUBLE_EQ(bb->price, 10.0);
  EXPECT_EQ(bb->agg_qty, 100);

  auto ba = ob.bestAsk();
  ASSERT_TRUE(ba.has_value());
  EXPECT_DOUBLE_EQ(ba->price, 11.0);
  EXPECT_EQ(ba->agg_qty, 200);
}

// ----- Cancel removes order and prunes empty level -----

TEST(OrderBookL3Test, CancelRemovesAndPrunesLevel) {
  const std::string csv =
      "NO,SECCODE,BUYSELL,TIME,ORDERNO,ACTION,PRICE,VOLUME,TRADENO,TRADEPRICE\n"
      "1,FOO,B,1000,1,1,10.00,100,,\n"
      "2,FOO,B,1001,2,9.50, 50,,,\n" // spurious 'trade' row w/o TRADENO will be ignored (unknown), harmless
      "3,FOO,B,1002,2,0,0,,,\n"      // also ignored
      "4,FOO,B,1003,2,0,0,,,\n"      // ignored
      "5,FOO,B,1004,0,0,0,0,,\n";    // ACTION=0 cancel order #1

  backtester::OrderBookL3 ob("FOO");
  ob.LoadCsv(WriteCsv("ob_cancel.csv", csv));

  // After cancel, bid side empty.
  EXPECT_FALSE(ob.bestBid().has_value());
}

// ----- Two-row trade fully consumes both sides -----

TEST(OrderBookL3Test, TradeTwoRowsFullFill) {
  const std::string csv =
      "NO,SECCODE,BUYSELL,TIME,ORDERNO,ACTION,PRICE,VOLUME,TRADENO,TRADEPRICE\n"
      "1,FOO,B,1000,1,1,10.00,100,,\n"     // add bid
      "2,FOO,S,1000,2,1,10.00,100,,\n"     // add ask
      // same TRADENO on both sides; each row reduces its own order;
      // we emit Trade only once (dedup by TRADENO)
      "3,FOO,B,1001,1,2,10.00,100,9001,10.00\n"
      "4,FOO,S,1001,2,2,10.00,100,9001,10.00\n";

  backtester::OrderBookL3 ob("FOO");
  ob.LoadCsv(WriteCsv("ob_trade_full.csv", csv));

  EXPECT_FALSE(ob.bestBid().has_value());
  EXPECT_FALSE(ob.bestAsk().has_value());
}

// ----- Partial fill across two trades leaves residual, then clears -----

TEST(OrderBookL3Test, TradePartialThenFinish) {
  const std::string csv =
      "NO,SECCODE,BUYSELL,TIME,ORDERNO,ACTION,PRICE,VOLUME,TRADENO,TRADEPRICE\n"
      "1,FOO,B,1000,1,1,10.00,150,,\n"     // add bid 150
      "2,FOO,S,1000,2,1,10.00,100,,\n"     // add ask 100
      // trade #1 consumes 100 on both sides
      "3,FOO,B,1001,1,2,10.00,100,9100,10.00\n"
      "4,FOO,S,1001,2,2,10.00,100,9100,10.00\n"
      // now bid #1 should have 50 left; add opposite for 50 and trade it
      "5,FOO,S,1002,3,1,10.00,50,,\n"      // add ask #3 for 50
      "6,FOO,B,1003,1,2,10.00,50,9101,10.00\n"
      "7,FOO,S,1003,3,2,10.00,50,9101,10.00\n";

  backtester::OrderBookL3 ob("FOO");
  ob.LoadCsv(WriteCsv("ob_trade_partial.csv", csv));

  // All cleared
  EXPECT_EQ(ob.openQty("1"), 0);
  EXPECT_FALSE(ob.bestBid().has_value());
  EXPECT_FALSE(ob.bestAsk().has_value());
}

// ----- For trades, if TRADEPRICE column is missing, fall back to PRICE -----

TEST(OrderBookL3Test, TradePriceFallbackWhenTRADEPRICEAbsent) {
  const std::string csv =
      // note: TRADEPRICE column omitted
      "NO,SECCODE,BUYSELL,TIME,ORDERNO,ACTION,PRICE,VOLUME,TRADENO\n"
      "1,FOO,B,1000,1,1,13.24,200,,\n"
      "2,FOO,S,1000,2,1,13.24,200,,\n"
      "3,FOO,B,1001,1,2,13.2415,200,777\n"
      "4,FOO,S,1001,2,2,13.2415,200,777\n";

  backtester::OrderBookL3 ob("FOO");
  ob.LoadCsv(WriteCsv("ob_trade_fallback.csv", csv));

  EXPECT_FALSE(ob.bestBid().has_value());
  EXPECT_FALSE(ob.bestAsk().has_value());
}

// ----- Only ingest target ticker -----

TEST(OrderBookL3Test, SkimSingleTickerOnly) {
  const std::string csv =
      "NO,SECCODE,BUYSELL,TIME,ORDERNO,ACTION,PRICE,VOLUME,TRADENO,TRADEPRICE\n"
      "1,BAR,B,1000,10,1,9.00,100,,\n"     // different ticker
      "2,FOO,B,1000,1,1,10.00,100,,\n"
      "3,BAR,S,1000,20,1,12.00,100,,\n"    // different ticker
      "4,FOO,S,1000,2,1,11.00,100,,\n";

  backtester::OrderBookL3 ob("FOO");
  ob.LoadCsv(WriteCsv("ob_skim.csv", csv));

  auto bb = ob.bestBid();
  ASSERT_TRUE(bb.has_value());
  EXPECT_DOUBLE_EQ(bb->price, 10.0);
  EXPECT_EQ(bb->agg_qty, 100);

  auto ba = ob.bestAsk();
  ASSERT_TRUE(ba.has_value());
  EXPECT_DOUBLE_EQ(ba->price, 11.0);
  EXPECT_EQ(ba->agg_qty, 100);
}

// ----- Iteration order: bids high->low, asks low->high -----

TEST(OrderBookL3Test, IterateDepthOrder) {
  const std::string csv =
      "NO,SECCODE,BUYSELL,TIME,ORDERNO,ACTION,PRICE,VOLUME,TRADENO,TRADEPRICE\n"
      "1,FOO,B,1000,1,1,10.0,10,,\n"
      "2,FOO,B,1000,2,1, 9.0,20,,\n"
      "3,FOO,S,1000,3,1,11.0,30,,\n"
      "4,FOO,S,1000,4,1,12.0,40,,\n";

  backtester::OrderBookL3 ob("FOO");
  ob.LoadCsv(WriteCsv("ob_iter.csv", csv));

  std::vector<double> bids;
  ob.ForEachLevel(backtester::OrderBookL3::Side::kBid,
      [&](const backtester::OrderBookL3::Level& lvl) { bids.push_back(lvl.price); });
  ASSERT_EQ(bids.size(), 2u);
  EXPECT_DOUBLE_EQ(bids[0], 10.0);
  EXPECT_DOUBLE_EQ(bids[1], 9.0);

  std::vector<double> asks;
  ob.ForEachLevel(backtester::OrderBookL3::Side::kAsk,
      [&](const backtester::OrderBookL3::Level& lvl) { asks.push_back(lvl.price); });
  ASSERT_EQ(asks.size(), 2u);
  EXPECT_DOUBLE_EQ(asks[0], 11.0);
  EXPECT_DOUBLE_EQ(asks[1], 12.0);
}

// ----- openQty reflects partial fills -----

TEST(OrderBookL3Test, OpenQtyAfterPartialFill) {
  const std::string csv =
      "NO,SECCODE,BUYSELL,TIME,ORDERNO,ACTION,PRICE,VOLUME,TRADENO,TRADEPRICE\n"
      "1,FOO,B,1000,1,1,10.00,120,,\n"         // add 120
      "2,FOO,S,1000,2,1,10.00, 80,,\n"         // add 80
      "3,FOO,B,1001,1,2,10.00, 80,9900,10.00\n"// trade 80 both sides
      "4,FOO,S,1001,2,2,10.00, 80,9900,10.00\n";

  backtester::OrderBookL3 ob("FOO");
  ob.LoadCsv(WriteCsv("ob_openqty.csv", csv));

  // 120-80 = 40 remaining on order #1
  EXPECT_EQ(ob.openQty("1"), 40);
  auto bb = ob.bestBid();
  ASSERT_TRUE(bb.has_value());
  EXPECT_DOUBLE_EQ(bb->price, 10.0);
  EXPECT_EQ(bb->agg_qty, 40);
}

// ----- Unknown action rows are skipped cleanly -----

TEST(OrderBookL3Test, UnknownActionSkipped) {
  const std::string csv =
      "NO,SECCODE,BUYSELL,TIME,ORDERNO,ACTION,PRICE,VOLUME,TRADENO,TRADEPRICE\n"
      "1,FOO,B,1000,1,1,10.0,100,,\n"      // add
      "2,FOO,B,1001,1, 9.0,200,,,\n"       // malformed -> skipped
      "3,FOO,B,1002,1, 0.0,0,,,\n"         // malformed -> skipped
      "4,FOO,S,1003,2,1,11.0,100,,,\n"     // add ask
      "5,FOO,B,1004,9,10.0,0,0,,,\n";      // ACTION=9 unknown

  backtester::OrderBookL3 ob("FOO");
  ob.LoadCsv(WriteCsv("ob_unknown_action.csv", csv));

  // Book should still reflect the two valid adds.
  EXPECT_TRUE(ob.bestBid().has_value());
  EXPECT_TRUE(ob.bestAsk().has_value());
}

// ----- Cancel of unknown order is a no-op -----

TEST(OrderBookL3Test, CancelUnknownIsNoop) {
  const std::string csv =
      "NO,SECCODE,BUYSELL,TIME,ORDERNO,ACTION,PRICE,VOLUME,TRADENO,TRADEPRICE\n"
      "1,FOO,B,1000,1,1,10.0,100,,\n"
      "2,FOO,B,1001,999999,0,0,0,0,,\n";  // cancel unknown

  backtester::OrderBookL3 ob("FOO");
  ob.LoadCsv(WriteCsv("ob_cancel_unknown.csv", csv));

  auto bb = ob.bestBid();
  ASSERT_TRUE(bb.has_value());
  EXPECT_DOUBLE_EQ(bb->price, 10.0);
  EXPECT_EQ(bb->agg_qty, 100);
}
