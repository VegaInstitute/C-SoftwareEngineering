#include "libbacktester/simulation_core.hpp"

#include <charconv>
#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace libbacktester {

//============================= Internal helpers =============================//
namespace {

struct CsvCols {
  int no = -1;         // NO
  int seccode = -1;    // SECCODE
  int buysell = -1;    // BUYSELL
  int time = -1;       // TIME (HHMMSSZZZ or HHMMSSZZZXXX, or integer ms)
  int orderno = -1;    // ORDERNO
  int action = -1;     // ACTION (0/1/2 or CANCEL/ADD/TRADE)
  int price = -1;      // PRICE
  int volume = -1;     // VOLUME
  int tradeno = -1;    // TRADENO (optional)
  int tradeprice = -1; // TRADEPRICE (optional)
};

constexpr char kCsvDelim = ',';

inline std::string_view Trim(std::string_view s) {
  while (!s.empty() && (s.front() == ' ' || s.front() == '\t' ||
                        s.front() == '\r' || s.front() == '\n')) {
    s.remove_prefix(1);
  }
  while (!s.empty() && (s.back() == ' ' || s.back() == '\t' ||
                        s.back() == '\r' || s.back() == '\n')) {
    s.remove_suffix(1);
  }
  return s;
}

inline std::vector<std::string_view> SplitCsv(std::string_view line) {
  std::vector<std::string_view> out;
  size_t start = 0;
  for (size_t i = 0; i <= line.size(); ++i) {
    if (i == line.size() || line[i] == kCsvDelim) {
      out.emplace_back(line.substr(start, i - start));
      start = i + 1;
    }
  }
  return out;
}

inline std::int64_t ToI64(std::string_view sv, std::int64_t def = 0) {
  sv = Trim(sv);
  if (sv.empty()) return def;
  std::int64_t v = def;
  auto* first = sv.data();
  auto* last  = sv.data() + sv.size();
  auto res = std::from_chars(first, last, v);
  return (res.ec == std::errc()) ? v : def;
}

inline double ToF64(std::string_view sv, double def = 0.0) {
  sv = Trim(sv);
  if (sv.empty()) return def;
  try { return std::stod(std::string{sv}); } catch (...) { return def; }
}

inline OrderBookL3::Action ParseActionToken(std::string_view sv) {
  sv = Trim(sv);
  if (sv == "1" || sv == "ADD" || sv == "Add" || sv == "add") return OrderBookL3::Action::kAdd;
  if (sv == "2" || sv == "TRADE" || sv == "Trade" || sv == "trade") return OrderBookL3::Action::kTrade;
  if (sv == "0" || sv == "CANCEL" || sv == "DEL" || sv == "Cancel" || sv == "cancel") return OrderBookL3::Action::kCancel;
  return OrderBookL3::Action::kUnknown;
}

inline OrderBookL3::Side ParseSideToken(std::string_view sv) {
  sv = Trim(sv);
  if (sv == "B" || sv == "b" || sv == "BUY" || sv == "Buy" || sv == "1") return OrderBookL3::Side::kBid;
  return OrderBookL3::Side::kAsk;
}

inline Ms ParseTimeMsFromString(std::string_view s) {
  const auto len = s.size();
  auto to_ms = [](int hh, int mm, int ss, int zzz) -> std::int64_t {
    return static_cast<std::int64_t>(hh) * 3'600'000LL +
           static_cast<std::int64_t>(mm) *   60'000LL +
           static_cast<std::int64_t>(ss) *    1'000LL +
           static_cast<std::int64_t>(zzz);
  };

  if (len == 9) {  // HHMMSSZZZ
    const int hh = std::stoi(std::string{s.substr(0, 2)});
    const int mm = std::stoi(std::string{s.substr(2, 2)});
    const int ss = std::stoi(std::string{s.substr(4, 2)});
    const int z  = std::stoi(std::string{s.substr(6, 3)});
    return Ms{to_ms(hh, mm, ss, z)};
  }
  if (len == 12) {  // HHMMSSZZZXXX (truncate µs to ms)
    const int hh = std::stoi(std::string{s.substr(0, 2)});
    const int mm = std::stoi(std::string{s.substr(2, 2)});
    const int ss = std::stoi(std::string{s.substr(4, 2)});
    const int z  = std::stoi(std::string{s.substr(6, 3)});
    return Ms{to_ms(hh, mm, ss, z)};
  }
  // integer milliseconds since midnight
  return Ms{ToI64(s, 0)};
}

inline std::unordered_map<std::string, int> BuildHeaderIndex(std::string_view header) {
  std::unordered_map<std::string, int> h2i;
  auto cols = SplitCsv(header);
  for (int i = 0; i < static_cast<int>(cols.size()); ++i) {
    h2i.emplace(std::string{cols[i]}, i);
  }
  return h2i;
}

inline int FindCol(const std::unordered_map<std::string, int>& h2i, std::string_view name) {
  auto it = h2i.find(std::string{name});
  return it == h2i.end() ? -1 : it->second;
}

struct MoexRow {
  Ms ts{0};
  std::uint64_t seq{0};
  std::string seccode;
  std::string order_id;
  OrderBookL3::Side side{OrderBookL3::Side::kBid};
  OrderBookL3::Action action{OrderBookL3::Action::kUnknown};
  double price{0.0};
  std::int64_t qty{0};
  std::string trade_id;
};

inline std::optional<MoexRow> ParseRow(std::string_view line, const CsvCols& c) {
  const auto f = SplitCsv(line);
  if (c.time < 0 || c.no < 0 || c.orderno < 0 || c.action < 0 ||
      c.price < 0 || c.volume < 0 || c.buysell < 0) {
    return std::nullopt;
  }
  if (static_cast<int>(f.size()) <= std::max({c.time, c.no, c.orderno, c.action,
                                              c.price, c.volume, c.buysell})) {
    return std::nullopt;
  }

  MoexRow r{};
  r.ts  = ParseTimeMsFromString(f[c.time]);
  r.seq = static_cast<std::uint64_t>(ToI64(f[c.no], 0));

  if (c.seccode >= 0 && c.seccode < static_cast<int>(f.size())) {
    auto sv = Trim(f[c.seccode]);
    r.seccode.assign(sv.begin(), sv.end());
  }

  {
    auto id = Trim(f[c.orderno]);
    r.order_id.assign(id.begin(), id.end());
  }

  r.action = ParseActionToken(f[c.action]);
  if (r.action == OrderBookL3::Action::kUnknown) return std::nullopt;

  r.side = ParseSideToken(f[c.buysell]);

  if (r.action == OrderBookL3::Action::kTrade &&
      c.tradeprice >= 0 && c.tradeprice < static_cast<int>(f.size())) {
    r.price = ToF64(f[c.tradeprice], 0.0);
  }
  if (r.price == 0.0) r.price = ToF64(f[c.price], 0.0);

  r.qty = ToI64(f[c.volume], 0);

  if (c.tradeno >= 0 && c.tradeno < static_cast<int>(f.size())) {
    auto tv = Trim(f[c.tradeno]);
    if (!tv.empty()) r.trade_id.assign(tv.begin(), tv.end());
  }

  return r;
}

}  // namespace
//===========================================================================//

SimulationCore::SimulationCore(OrderBookL3* book,
                               Strategy* strategy,
                               ExecutionSimulator* exec,
                               Portfolio* portfolio,
                               MetricsWriter* metrics) noexcept
    : book_(book),
      strategy_(strategy),
      exec_(exec),
      portfolio_(portfolio),
      metrics_(metrics),
      now_(Ms{0}),
      next_timer_ts_(Ms{0}) {}

Ms SimulationCore::parseMoexTimeMs_(const std::string& s) {
  return ParseTimeMsFromString(s);
}

OrderBookL3::Action SimulationCore::toAction_(int a) {
  switch (a) {
    case 0: return OrderBookL3::Action::kCancel;
    case 1: return OrderBookL3::Action::kAdd;
    case 2: return OrderBookL3::Action::kTrade;
    default: return OrderBookL3::Action::kUnknown;
  }
}

OrderBookL3::Side SimulationCore::toSide_(char c) {
  return (c == 'B' || c == 'b' || c == '1') ? OrderBookL3::Side::kBid
                                            : OrderBookL3::Side::kAsk;
}

void SimulationCore::fireTimerTick_(Ms ts) {
  now_ = ts;
  strategy_->onTimer(ts);

  auto intents = strategy_->popPendingIntents();
  exec_->submitIntents(std::move(intents), ts);

  // Metrics/mark-to-market are optional if metrics_ is null.
  if (portfolio_) {
    double bid = 0.0, ask = 0.0;
    if (auto bb = book_->bestBid()) bid = bb->price;
    if (auto aa = book_->bestAsk()) ask = aa->price;
    portfolio_->markToMarket(bid, ask);
    if (metrics_) metrics_->recordEquity(ts, portfolio_->equity());
  }
}

void SimulationCore::run(const std::string& csv_path, Ms timer_step) {
  std::ifstream in(csv_path);
  if (!in.is_open()) {
    throw std::runtime_error("SimulationCore: cannot open CSV: " + csv_path);
  }

  std::string header_line;
  if (!std::getline(in, header_line)) return;

  const auto h2i = BuildHeaderIndex(header_line);

  CsvCols c;
  c.no         = FindCol(h2i, "NO");
  c.seccode    = FindCol(h2i, "SECCODE");
  c.buysell    = FindCol(h2i, "BUYSELL");
  c.time       = FindCol(h2i, "TIME");
  c.orderno    = FindCol(h2i, "ORDERNO");
  c.action     = FindCol(h2i, "ACTION");
  c.price      = FindCol(h2i, "PRICE");
  c.volume     = FindCol(h2i, "VOLUME");
  c.tradeno    = FindCol(h2i, "TRADENO");
  c.tradeprice = FindCol(h2i, "TRADEPRICE");

  if (c.no < 0 || c.time < 0 || c.orderno < 0 || c.action < 0 ||
      c.price < 0 || c.volume < 0 || c.buysell < 0) {
    throw std::runtime_error("SimulationCore: CSV missing required MOEX columns.");
  }

  std::string seccode_filter;

  // Prime with first accepted row.
  std::optional<MoexRow> cur;
  {
    std::string line;
    while (std::getline(in, line)) {
      auto r = ParseRow(line, c);
      if (!r) continue;
      if (!r->seccode.empty()) {
        if (seccode_filter.empty()) seccode_filter = r->seccode;
        if (r->seccode != seccode_filter) continue;
      }
      cur = std::move(r);
      break;
    }
    if (!cur) return;
  }

  const auto rem = cur->ts.count() % timer_step.count();
  next_timer_ts_ = Ms{cur->ts.count() - rem};

  // Prefetch
  std::optional<MoexRow> next;
  {
    std::string line;
    while (std::getline(in, line)) {
      auto r = ParseRow(line, c);
      if (!r) continue;
      if (!seccode_filter.empty() && !r->seccode.empty() &&
          r->seccode != seccode_filter) continue;
      next = std::move(r);
      break;
    }
  }

  // Main loop
  while (true) {
    while (next_timer_ts_ < cur->ts) {
      fireTimerTick_(next_timer_ts_);
      next_timer_ts_ += timer_step;
    }

    // Process current ts (and all rows with same ts)
    {
      OrderBookL3::OrderKey rec{};
      rec.order_id = cur->order_id;
      rec.side     = cur->side;
      rec.price    = cur->price;
      rec.qty      = cur->qty;
      rec.ts_ms    = cur->ts.count();
      rec.seq      = cur->seq;
      (void)book_->onRecord(rec, cur->action, cur->trade_id.empty() ? nullptr : &cur->trade_id);
      strategy_->onBookUpdate(cur->ts, cur->seq);

      while (next && next->ts == cur->ts) {
        OrderBookL3::OrderKey r2{};
        r2.order_id = next->order_id;
        r2.side     = next->side;
        r2.price    = next->price;
        r2.qty      = next->qty;
        r2.ts_ms    = next->ts.count();
        r2.seq      = next->seq;
        (void)book_->onRecord(r2, next->action, next->trade_id.empty() ? nullptr : &next->trade_id);
        strategy_->onBookUpdate(next->ts, next->seq);

        // prefetch again
        std::string line;
        next.reset();
        while (std::getline(in, line)) {
          auto r = ParseRow(line, c);
          if (!r) continue;
          if (!seccode_filter.empty() && !r->seccode.empty() &&
              r->seccode != seccode_filter) continue;
          next = std::move(r);
          break;
        }
      }
    }

    if (next_timer_ts_ == cur->ts) {
      fireTimerTick_(cur->ts);
      next_timer_ts_ += timer_step;
    }

    if (!next) break;
    cur = std::move(next);

    std::string line;
    next.reset();
    while (std::getline(in, line)) {
      auto r = ParseRow(line, c);
      if (!r) continue;
      if (!seccode_filter.empty() && !r->seccode.empty() &&
          r->seccode != seccode_filter) continue;
      next = std::move(r);
      break;
    }
  }
}

}  // namespace libbacktester
