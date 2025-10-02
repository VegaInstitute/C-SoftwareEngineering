#ifndef BACKTESTER_ORDERBOOK_L3_H_
#define BACKTESTER_ORDERBOOK_L3_H_

#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace backtester {

// Single-instrument L3 order book for MOEX-like ORDLOG (Type A).
class OrderBookL3 {
 public:
  enum class Action : std::uint8_t { kAdd, kCancel, kTrade, kReplace, kUnknown };
  enum class Side   : std::uint8_t { kBid, kAsk };

  struct OrderKey {
    std::string  order_id;
    Side         side{Side::kBid};
    double       price{0.0};
    std::int64_t qty{0};
    std::int64_t ts_ms{0};
    std::uint64_t seq{0};
  };

  struct Trade {
    std::string  trade_id;
    double       price{0.0};
    std::int64_t qty{0};
    std::int64_t ts_ms{0};
    std::uint64_t seq{0};
  };

  struct Level {
    double       price{0.0};
    std::int64_t agg_qty{0};
    std::size_t  count{0};
  };

  explicit OrderBookL3(std::string ticker);
  ~OrderBookL3();

  OrderBookL3(const OrderBookL3&)            = delete;
  OrderBookL3& operator=(const OrderBookL3&) = delete;
  OrderBookL3(OrderBookL3&&) noexcept        = default;
  OrderBookL3& operator=(OrderBookL3&&) noexcept = default;

  // Process one decoded event. On trades, reductions ALWAYS occur;
  // duplicate trade rows are suppressed only for emission (by TRADENO).
  std::optional<Trade> OnRecord(const OrderKey& rec,
                                Action action,
                                const std::string* trade_id = nullptr);

  // Load a MOEX-style CSV and ingest only rows for this book's ticker.
  void LoadCsv(const std::string& filepath);

  // Top-of-book helpers.
  std::optional<Level> bestBid() const;
  std::optional<Level> bestAsk() const;

  // Depth iteration (bids: high->low, asks: low->high).
  void ForEachLevel(Side side, const std::function<void(const Level&)>& fn) const;

  // Remaining open quantity of a known order id (0 if absent).
  std::int64_t openQty(const std::string& order_id) const;

 private:
  struct BookOrder {
    std::string  order_id;
    std::int64_t qty{0};
    std::int64_t ts_ms{0};
    std::uint64_t seq{0};
  };
  using PriceLevel = std::list<BookOrder>;

  std::string ticker_;                           // instrument this book tracks
  std::map<double, PriceLevel> bids_;            // ascending price
  std::map<double, PriceLevel> asks_;            // ascending price

  struct IndexEntry {
    Side                 side;
    double               price;
    PriceLevel*          level;
    PriceLevel::iterator iter;
  };
  std::unordered_map<std::string, IndexEntry> index_;       // ORDERNO -> location
  std::unordered_set<std::string>             seen_trades_; // TRADENO dedup (emission)
  std::uint64_t                                seq_counter_{1};

  // CSV helpers
  static std::string_view Trim(std::string_view s);
  static std::string NormalizeId(std::string_view s);  // trim + strip leading zeros
  static std::vector<std::string_view> SplitLine(std::string_view line, char delim);
  static std::unordered_map<std::string, int>
      BuildHeaderIndex(const std::vector<std::string_view>& hdrs);

  // Book ops
  void AddOrder(const OrderKey& rec);
  void CancelOrder(const std::string& order_id);
  void ReduceOrder(const std::string& order_id, std::int64_t qty);

  // Queries
  std::optional<Level> BestFromBook(const std::map<double, PriceLevel>& book,
                                    bool highest) const;
  void AccumulateLevel(const PriceLevel& lvl, Level& out) const;
};

} // namespace backtester

#endif // BACKTESTER_ORDERBOOK_L3_H_
