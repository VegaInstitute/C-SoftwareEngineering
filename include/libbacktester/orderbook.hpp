#ifndef LIBBACKTESTER_ORDERBOOK_L3_HPP_
#define LIBBACKTESTER_ORDERBOOK_L3_HPP_

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

namespace libbacktester {

class OrderBookL3 {
  public:
  enum class Action : std::uint8_t { kAdd, kCancel, kTrade, kReplace, kUnknown };
  enum class Side   : std::uint8_t { kBid, kAsk };

  struct OrderKey {
    std::string   order_id;
    Side          side{Side::kBid};
    double        price{0.0};
    std::int64_t  qty{0};
    std::int64_t  ts_ms{0};
    std::uint64_t seq{0};
  };

  struct Trade {
    std::string   trade_id;
    double        price{0.0};
    std::int64_t  qty{0};
    std::int64_t  ts_ms{0};
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

  // Process one decoded event. For Action::kTrade, reductions ALWAYS occur.
  // Duplicate trade rows are suppressed for emission only (dedup by trade_id).
  std::optional<Trade> onRecord(const OrderKey& rec,
                                Action action,
                                const std::string* trade_id = nullptr);

  // DEPRECATED. Load a MOEX-style CSV and ingest only rows for this book's ticker.
  [[deprecated("The functionality is implemented in the SimulationCore class.")]] void loadCsv(const std::string& filepath);

  // Top-of-book.
  std::optional<Level> bestBid() const;
  std::optional<Level> bestAsk() const;

  // Depth iteration (bids: high->low, asks: low->high).
  void forEachLevel(Side side, const std::function<void(const Level&)>& fn) const;

  // Remaining open quantity of a known order id (0 if absent).
  std::int64_t openQty(const std::string& order_id) const;

private:
  struct BookOrder {
    std::string   order_id;
    std::int64_t  qty{0};
    std::int64_t  ts_ms{0};
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
  static std::string_view trim(std::string_view s);
  static std::string normalizeId(std::string_view s);  // trim + strip leading zeros
  static bool isZeroLikeId(std::string_view s);        // "", "0", "0000", ...
  static std::vector<std::string_view> splitLine(std::string_view line, char delim);
  static std::unordered_map<std::string, int> buildHeaderIndex(const std::vector<std::string_view>& hdrs);

  // Book ops
  void addOrder(const OrderKey& rec);
  void cancelOrder(const std::string& order_id);
  void cancelAll(Side side);  // blanket side cancel
  void reduceOrder(const std::string& order_id, std::int64_t qty);

  // Queries
  std::optional<Level> bestFromBook(const std::map<double, PriceLevel>& book,
                                    bool highest) const;
  void accumulateLevel(const PriceLevel& lvl, Level& out) const;
};

} // namespace libbacktester
#endif // LIBBACKTESTER_ORDERBOOK_L3_HPP_
