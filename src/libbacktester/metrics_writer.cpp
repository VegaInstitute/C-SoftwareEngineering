#include "libbacktester/metrics_writer.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace libbacktester {
namespace {

const char* SideToStr(OrderSide s) {
  return (s == OrderSide::kBuy) ? "Buy" : "Sell";
}

}  // namespace

MetricsWriter* MetricsWriter::singleton_ = nullptr;

MetricsWriter::MetricsWriter(std::string out_dir)
    : out_dir_(std::move(out_dir)),
      trades_path_(std::filesystem::path(out_dir_) / "trades.csv"),
      equity_path_(std::filesystem::path(out_dir_) / "equity.csv") {
  std::filesystem::create_directories(out_dir_);

  // Create/truncate files and write headers; flush so tests can read immediately.
  {
    std::ofstream t(trades_path_, std::ios::out | std::ios::trunc);
    if (!t) throw std::runtime_error("Failed to open trades.csv for write");
    t << "ts_ms,side,qty,price,strategy_id\n";
    t.flush();
  }
  {
    std::ofstream e(equity_path_, std::ios::out | std::ios::trunc);
    if (!e) throw std::runtime_error("Failed to open equity.csv for write");
    e << "ts_ms,equity\n";
    e.flush();
  }
}

void MetricsWriter::init(const std::string& out_dir) {
  if (singleton_ != nullptr) return;
  singleton_ = new MetricsWriter(out_dir);
}

MetricsWriter& MetricsWriter::instance() {
  if (singleton_ == nullptr) {
    throw std::logic_error("MetricsWriter::init() must be called first");
  }
  return *singleton_;
}

void MetricsWriter::recordTrade(const FillEvent& fill) {
  std::cerr << "[MetricsWriter] recordTrade ts=" << fill.ts.count()
            << " side=" << (fill.side == OrderSide::kBuy ? "Buy" : "Sell")
            << " qty=" << fill.qty << " px=" << fill.price
            << " strategy_id=" << fill.strategy_id << "\n";

  std::lock_guard<std::mutex> lock(mtx_);

  trades_buf_.push_back(
      TradeRec{fill.ts, fill.side, fill.qty, fill.price, fill.strategy_id});

  // Keep buffer sorted by timestamp (stable for equal ts).
  std::stable_sort(trades_buf_.begin(), trades_buf_.end(),
                   [](const TradeRec& a, const TradeRec& b) {
                     return a.ts.count() < b.ts.count();
                   });

  writeTradesLocked();
}

void MetricsWriter::recordEquity(Ms ts, double equity) {
  std::cerr << "[MetricsWriter] recordEquity ts=" << ts.count()
            << " equity=" << equity << "\n";
  std::lock_guard<std::mutex> lock(mtx_);

  equity_buf_.push_back(EquityRec{ts, equity});
  std::stable_sort(equity_buf_.begin(), equity_buf_.end(),
                   [](const EquityRec& a, const EquityRec& b) {
                     return a.ts.count() < b.ts.count();
                   });

  writeEquityLocked();
}

void MetricsWriter::writeTradesLocked() {
  std::ofstream t(trades_path_, std::ios::out | std::ios::trunc);
  if (!t) throw std::runtime_error("Failed to open trades.csv for rewrite");

  t << "ts_ms,side,qty,price,strategy_id\n";
  for (const auto& r : trades_buf_) {
    t << r.ts.count() << ',' << SideToStr(r.side) << ',' << r.qty << ','
      << std::setprecision(12) << r.price << ',' << r.strategy_id << '\n';
  }
  t.flush();
}

void MetricsWriter::writeEquityLocked() {
  std::ofstream e(equity_path_, std::ios::out | std::ios::trunc);
  if (!e) throw std::runtime_error("Failed to open equity.csv for rewrite");

  e << "ts_ms,equity\n";
  for (const auto& r : equity_buf_) {
    e << r.ts.count() << ',' << std::setprecision(12) << r.equity << '\n';
  }
  e.flush();
}

}  // namespace libbacktester
