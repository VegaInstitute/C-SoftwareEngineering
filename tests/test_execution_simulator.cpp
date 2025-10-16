#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <vector>

#include "libbacktester/metrics_writer.hpp"
#include "libbacktester/types.hpp"

using libbacktester::FillEvent;
using libbacktester::MetricsWriter;
using libbacktester::Ms;
using libbacktester::OrderSide;

namespace {

std::vector<std::string> ReadAll(const std::string& path) {
  std::ifstream f(path);
  std::vector<std::string> lines;
  std::string s;
  while (std::getline(f, s)) lines.push_back(s);
  return lines;
}

class MetricsFixture : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    static bool inited = false;
    if (!inited) {
      MetricsWriter::init("qa_metrics_out");
      inited = true;
    }
  }
};

}  // namespace

TEST_F(MetricsFixture, HeadersExistAfterInit) {
  auto tlines = ReadAll("qa_metrics_out/trades.csv");
  auto elines = ReadAll("qa_metrics_out/equity.csv");
  ASSERT_FALSE(tlines.empty());
  ASSERT_FALSE(elines.empty());
  EXPECT_EQ(tlines[0], "ts_ms,side,qty,price,strategy_id");
  EXPECT_EQ(elines[0], "ts_ms,equity");
}

TEST_F(MetricsFixture, AppendsAreChronologicallyOrdered) {
  MetricsWriter& W = MetricsWriter::instance();

  // Intentionally out-of-order; files must be sorted by timestamp.
  FillEvent f2{}; f2.strategy_id = "b"; f2.side = OrderSide::kBuy;  f2.qty = 1; f2.price = 10; f2.ts = Ms{2000};
  FillEvent f1{}; f1.strategy_id = "a"; f1.side = OrderSide::kSell; f1.qty = 2; f1.price = 11; f1.ts = Ms{1000};
  W.recordTrade(f2);
  W.recordTrade(f1);

  auto tlines = ReadAll("qa_metrics_out/trades.csv");
  ASSERT_GE(tlines.size(), 3u);

  long long prev_ts = -1;
  for (size_t i = 1; i < tlines.size(); ++i) {
    const auto& line = tlines[i];
    const auto comma = line.find(',');
    ASSERT_NE(comma, std::string::npos);
    const long long ts_val = std::stoll(line.substr(0, comma));
    EXPECT_GE(ts_val, prev_ts) << "Trades must be sorted by timestamp";
    prev_ts = ts_val;
  }

  // Equity as well.
  W.recordEquity(Ms{2000}, 600.0);
  W.recordEquity(Ms{1000}, 500.0);

  auto elines = ReadAll("qa_metrics_out/equity.csv");
  ASSERT_GE(elines.size(), 3u);
  prev_ts = -1;
  for (size_t i = 1; i < elines.size(); ++i) {
    const auto& line = elines[i];
    const auto comma = line.find(',');
    ASSERT_NE(comma, std::string::npos);
    const long long ts_val = std::stoll(line.substr(0, comma));
    EXPECT_GE(ts_val, prev_ts) << "Equity must be sorted by timestamp";
    prev_ts = ts_val;
  }
}

TEST_F(MetricsFixture, TradeLineSchemaIsParsable) {
  MetricsWriter& W = MetricsWriter::instance();
  FillEvent f{}; f.strategy_id = "s"; f.side = OrderSide::kBuy; f.qty = 7; f.price = 123.45; f.ts = Ms{3000};
  W.recordTrade(f);

  auto tlines = ReadAll("qa_metrics_out/trades.csv");
  ASSERT_FALSE(tlines.empty());
  const auto& last = tlines.back();

  // Expect 5 CSV columns (4 commas).
  int commas = 0;
  for (char c : last) if (c == ',') ++commas;
  EXPECT_EQ(commas, 4);
}
