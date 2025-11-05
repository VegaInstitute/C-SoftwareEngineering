#include <charconv>
#include <fstream>
#include <stdexcept>
#include <string>

#include "libbacktester/orderbook.hpp"

namespace libbacktester {

// ------------------ ctor/dtor ------------------

OrderBookL3::OrderBookL3(std::string ticker) : ticker_(std::move(ticker)) {}
OrderBookL3::~OrderBookL3() = default;

// ------------------ public API ------------------

std::optional<OrderBookL3::Trade> OrderBookL3::onRecord(
    const OrderKey& rec, Action action, const std::string* trade_id) {
  switch (action) {
    case Action::kAdd:
      // Guard against zero/negative price or qty for add.
      if (rec.qty > 0 && rec.price > 0.0) {
        addOrder(rec);
      }
      return std::nullopt;

    case Action::kCancel: {
      // If ORDERNO is zero-like => blanket side cancel.
      if (isZeroLikeId(rec.order_id)) {
        cancelAll(rec.side);
        return std::nullopt;
      }
      // Targeted cancel by id.
      cancelOrder(rec.order_id);
      return std::nullopt;
    }

    case Action::kTrade: {
      // ALWAYS reduce this row's order (both trade rows reduce their own side).
      if (rec.qty > 0) {
        reduceOrder(rec.order_id, rec.qty);
      }

      // Dedup only affects emission, not reduction.
      if (trade_id && !trade_id->empty()) {
        const bool first_time = seen_trades_.insert(*trade_id).second;
        if (!first_time) return std::nullopt;

        Trade t;
        t.trade_id = *trade_id;
        t.price    = rec.price;
        t.qty      = rec.qty;
        t.ts_ms    = rec.ts_ms;
        t.seq      = rec.seq ? rec.seq : (seq_counter_++);
        return t;
      }
      return std::nullopt;
    }

    case Action::kReplace:
      // Not implemented in this simplified build.
      return std::nullopt;

    default:
      return std::nullopt;
  }
}

[[deprecated]] void OrderBookL3::loadCsv(const std::string& filepath) {
  // Common MOEX ORDLOG (Type A) columns:
  // NO,SECCODE,BUYSELL,TIME,ORDERNO,ACTION,PRICE,VOLUME,TRADENO,TRADEPRICE
  // TRADEPRICE may be absent; fall back to PRICE.

  constexpr char kDelim = ',';
  const std::string kColSeccode    = "SECCODE";
  const std::string kColTime       = "TIME";
  const std::string kColAction     = "ACTION";
  const std::string kColOrderno    = "ORDERNO";
  const std::string kColSide       = "BUYSELL";
  const std::string kColPrice      = "PRICE";
  const std::string kColQty        = "VOLUME";
  const std::string kColTradeId    = "TRADENO";
  const std::string kColTradePrice = "TRADEPRICE";

  auto parse_action = [](std::string_view sv) -> Action {
    sv = trim(sv);
    if (sv == "1" || sv == "ADD" || sv == "Add")       return Action::kAdd;
    if (sv == "2" || sv == "TRADE" || sv == "Trade")   return Action::kTrade;
    if (sv == "0" || sv == "CANCEL" || sv == "DEL")    return Action::kCancel;
    return Action::kUnknown;
  };
  auto parse_side = [](std::string_view sv) -> Side {
    sv = trim(sv);
    if (sv == "B" || sv == "BUY" || sv == "Buy" || sv == "1") return Side::kBid;
    return Side::kAsk;
  };
  auto to_i64 = [](std::string_view sv, std::int64_t& out) -> bool {
    sv = trim(sv);
    auto r = std::from_chars(sv.data(), sv.data() + sv.size(), out);
    return r.ec == std::errc();
  };
  auto to_f64 = [](std::string_view sv, double& out) -> bool {
    sv = trim(sv);
    auto r = std::from_chars(sv.data(), sv.data() + sv.size(), out);
    return r.ec == std::errc();
  };

  std::ifstream in(filepath);
  if (!in.is_open()) {
    throw std::runtime_error("Unable to open CSV: " + filepath);
  }

  std::string header_line;
  if (!std::getline(in, header_line)) return;
  auto hdr    = splitLine(header_line, kDelim);
  auto hindex = buildHeaderIndex(hdr);

  auto get_req = [&](const std::string& nm) -> int {
    auto it = hindex.find(nm);
    if (it == hindex.end()) throw std::runtime_error("Missing header: " + nm);
    return it->second;
  };
  auto get_opt = [&](const std::string& nm) -> int {
    auto it = hindex.find(nm);
    return (it == hindex.end() ? -1 : it->second);
  };

  const int i_seccode    = get_opt(kColSeccode);
  const int i_time       = get_req(kColTime);
  const int i_action     = get_req(kColAction);
  const int i_orderno    = get_req(kColOrderno);
  const int i_side       = get_req(kColSide);
  const int i_price      = get_req(kColPrice);
  const int i_qty        = get_req(kColQty);
  const int i_trade_id   = get_opt(kColTradeId);
  const int i_trade_px   = get_opt(kColTradePrice);

  std::string line;
  while (std::getline(in, line)) {
    if (line.empty()) continue;
    auto f = splitLine(line, kDelim);

    if (i_seccode >= 0) {
      if (trim(f[i_seccode]) != std::string_view{ticker_}) continue;
    }

    const Action act = parse_action(f[i_action]);
    if (act == Action::kUnknown) continue;

    OrderKey rec{};
    (void)to_i64(f[i_time], rec.ts_ms);
    rec.order_id = normalizeId(f[i_orderno]);  // trim + strip leading zeros
    rec.side     = parse_side(f[i_side]);

    // qty
    (void)to_i64(f[i_qty], rec.qty);

    // price (TRADEPRICE preferred for trades if present)
    double px = 0.0;
    if (act == Action::kTrade && i_trade_px >= 0) {
      if (!to_f64(f[i_trade_px], px)) px = 0.0;
    }
    if (px == 0.0) {
      (void)to_f64(f[i_price], px);
    }
    rec.price = px;

    std::string trade_id;
    if (i_trade_id >= 0) {
      std::string_view tv = trim(f[i_trade_id]);
      if (!tv.empty()) trade_id.assign(tv.begin(), tv.end());
    }

    (void)onRecord(rec, act, (i_trade_id >= 0 ? &trade_id : nullptr));
  }
}

std::optional<OrderBookL3::Level> OrderBookL3::bestBid() const {
  return bestFromBook(bids_, /*highest=*/true);
}

std::optional<OrderBookL3::Level> OrderBookL3::bestAsk() const {
  return bestFromBook(asks_, /*highest=*/false);
}

void OrderBookL3::forEachLevel(
    Side side, const std::function<void(const Level&)>& fn) const {
  if (side == Side::kBid) {
    for (auto it = bids_.rbegin(); it != bids_.rend(); ++it) {
      Level lvl{it->first, 0, it->second.size()};
      accumulateLevel(it->second, lvl);
      fn(lvl);
    }
  } else {
    for (const auto& [px, lst] : asks_) {
      Level lvl{px, 0, lst.size()};
      accumulateLevel(lst, lvl);
      fn(lvl);
    }
  }
}

std::int64_t OrderBookL3::openQty(const std::string& order_id) const {
  auto it = index_.find(order_id);
  return (it == index_.end()) ? 0 : it->second.iter->qty;
}

// ------------------ private helpers ------------------

std::string_view OrderBookL3::trim(std::string_view s) {
  auto is_space = [](char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
  };
  std::size_t b = 0;
  while (b < s.size() && is_space(s[b])) ++b;
  std::size_t e = s.size();
  while (e > b && is_space(s[e - 1])) --e;
  return s.substr(b, e - b);
}

std::string OrderBookL3::normalizeId(std::string_view s) {
  s = trim(s);
  // Strip leading zeros while keeping one '0' if the id is all zeros.
  std::size_t i = 0;
  while (i + 1 < s.size() && s[i] == '0') ++i;
  std::string out(s.substr(i));
  if (out.empty()) out = "0";
  return out;
}

bool OrderBookL3::isZeroLikeId(std::string_view s) {
  s = trim(s);
  if (s.empty()) return true;
  for (char c : s) {
    if (c != '0') return false;
  }
  return true;
}

std::vector<std::string_view> OrderBookL3::splitLine(std::string_view line, char delim) {
  std::vector<std::string_view> out;
  out.reserve(16);
  const char* p   = line.data();
  const size_t n  = line.size();
  size_t start = 0;
  for (size_t i = 0; i < n; ++i) {
    if (p[i] == delim) {
      out.emplace_back(p + start, i - start);
      start = i + 1;
    }
  }
  out.emplace_back(p + start, n - start);
  return out;
}

std::unordered_map<std::string, int>
OrderBookL3::buildHeaderIndex(const std::vector<std::string_view>& hdrs) {
  std::unordered_map<std::string, int> m;
  m.reserve(hdrs.size());
  for (int i = 0; i < static_cast<int>(hdrs.size()); ++i) {
    std::string_view k = trim(hdrs[i]);
    m.emplace(std::string(k), i);
  }
  return m;
}

void OrderBookL3::addOrder(const OrderKey& rec) {
  auto& book = (rec.side == Side::kBid ? bids_ : asks_);
  auto& lvl  = book[rec.price];
  lvl.push_back(BookOrder{rec.order_id, rec.qty, rec.ts_ms, rec.seq});
  auto it = std::prev(lvl.end());  // iterator to inserted element
  index_[rec.order_id] = IndexEntry{rec.side, rec.price, &lvl, it};
}

void OrderBookL3::cancelOrder(const std::string& order_id) {
  auto it = index_.find(order_id);
  if (it == index_.end()) return;

  IndexEntry e = it->second;   // capture before erasing from map
  e.level->erase(e.iter);      // remove node from the list
  if (e.level->empty()) {
    if (e.side == Side::kBid) {
      auto m_it = bids_.find(e.price);
      if (m_it != bids_.end() && m_it->second.empty()) bids_.erase(m_it);
    } else {
      auto m_it = asks_.find(e.price);
      if (m_it != asks_.end() && m_it->second.empty()) asks_.erase(m_it);
    }
  }
  index_.erase(it);
}

void OrderBookL3::cancelAll(Side side) {
  // Collect ids first to avoid iterator invalidation during erases.
  std::vector<std::string> ids;
  ids.reserve(index_.size());
  for (const auto& kv : index_) {
    if (kv.second.side == side) ids.push_back(kv.first);
  }
  for (const auto& id : ids) {
    cancelOrder(id);
  }
}

void OrderBookL3::reduceOrder(const std::string& order_id, std::int64_t qty) {
  auto it = index_.find(order_id);
  if (it == index_.end()) return;

  IndexEntry& e = it->second;
  if (qty >= e.iter->qty) {
    cancelOrder(order_id);
  } else if (qty > 0) {
    e.iter->qty -= qty;
  }
}

std::optional<OrderBookL3::Level> OrderBookL3::bestFromBook(
    const std::map<double, PriceLevel>& book, bool highest) const {
  if (book.empty()) return std::nullopt;
  const auto it = highest ? std::prev(book.end()) : book.begin();
  Level lvl{it->first, 0, it->second.size()};
  accumulateLevel(it->second, lvl);
  return lvl;
}

void OrderBookL3::accumulateLevel(const PriceLevel& lvl, Level& out) const {
  out.agg_qty = 0;
  for (const auto& o : lvl) out.agg_qty += o.qty;
}

}  // namespace libbacktester
