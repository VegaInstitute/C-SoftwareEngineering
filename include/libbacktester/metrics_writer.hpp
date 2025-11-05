#ifndef LIBBACKTESTER_METRICS_WRITER_HPP_
#define LIBBACKTESTER_METRICS_WRITER_HPP_

#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

#include "libbacktester/types.hpp"  // Ms, FillEvent, OrderSide

namespace libbacktester {

/**
 * @class MetricsWriter
 * @brief CSV writer for trades and equity; guarantees chronological order.
 *
 * Files in out_dir:
 *  - trades.csv : "ts_ms,side,qty,price,strategy_id"
 *  - equity.csv : "ts_ms,equity"
 *
 * Contract:
 *  - Files exist with headers after init().
 *  - On every record*, on-disk CSVs are non-decreasing by timestamp.
 *  - Implementation keeps in-memory buffers and rewrites sorted files.
 */
class MetricsWriter {
 public:
  MetricsWriter(const MetricsWriter&) = delete;
  MetricsWriter& operator=(const MetricsWriter&) = delete;

  /** Initialize the singleton with an output directory (idempotent). */
  static void init(const std::string& out_dir);

  /** Access the singleton instance (requires prior init()). */
  static MetricsWriter& instance();

  /** Append one trade; on-disk CSV remains sorted by timestamp. */
  void recordTrade(const FillEvent& fill);

  /** Append one equity snapshot; on-disk CSV remains sorted by timestamp. */
  void recordEquity(Ms ts, double equity);

 private:
  explicit MetricsWriter(std::string out_dir);

  struct TradeRec {
    Ms ts;
    OrderSide side;
    std::int64_t qty;
    double price;
    std::string strategy_id;
  };

  struct EquityRec {
    Ms ts;
    double equity;
  };

  std::string out_dir_;
  std::filesystem::path trades_path_;
  std::filesystem::path equity_path_;

  std::vector<TradeRec> trades_buf_;
  std::vector<EquityRec> equity_buf_;

  std::mutex mtx_;

  void writeTradesLocked();
  void writeEquityLocked();

  static MetricsWriter* singleton_;
};

}  // namespace libbacktester

#endif  // LIBBACKTESTER_METRICS_WRITER_HPP_
