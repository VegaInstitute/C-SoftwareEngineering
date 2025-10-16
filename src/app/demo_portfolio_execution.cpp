#include <iostream>
#include <string>

#include "libbacktester/orderbook.hpp"
#include "libbacktester/types.hpp"
#include "libbacktester/strategy.hpp"
#include "libbacktester/metrics_writer.hpp"
#include "libbacktester/portfolio.hpp"
#include "libbacktester/execution_simulator.hpp"
#include "libbacktester/strategies/twap_strategy.hpp"

namespace {

using libbacktester::Ms;
using libbacktester::OrderBookL3;
using libbacktester::OrderSide;

// Small helper to build OrderBookL3::OrderKey records.
OrderBookL3::OrderKey makeRec(const std::string& id,
                              OrderBookL3::Side side,
                              double px,
                              std::int64_t qty,
                              std::int64_t ts_ms,
                              std::uint64_t seq) {
  OrderBookL3::OrderKey k;
  k.order_id = id;
  k.side = side;
  k.price = px;
  k.qty = qty;
  k.ts_ms = ts_ms;
  k.seq = seq;
  return k;
}

// Seed a minimal book with a tight spread.
struct SeedState {
  std::string bid_id;
  std::string ask_id;
  std::uint64_t next_seq{1};
};

SeedState seedBook(OrderBookL3& book) {
  std::uint64_t seq = 1;
  book.onRecord(makeRec("B0", OrderBookL3::Side::kBid, 100.00, 1'000, 0, seq++),
                OrderBookL3::Action::kAdd);
  book.onRecord(makeRec("A0", OrderBookL3::Side::kAsk, 100.20, 1'000, 0, seq++),
                OrderBookL3::Action::kAdd);
  return {"B0", "A0", seq};
}

// Every 10s, cancel old quotes and place new ones slightly shifted.
void stepQuotes(OrderBookL3& book, SeedState& s, Ms now) {
  const auto bb = book.bestBid();
  const auto ba = book.bestAsk();
  if (!bb || !ba) return;

  // Shift bid up by 0.05 and keep 20 cents spread.
  const double new_bid = bb->price + 0.05;
  const double new_ask = new_bid + 0.20;

  // Cancel prior top quotes.
  book.onRecord(makeRec(s.bid_id, OrderBookL3::Side::kBid, bb->price, 0, now.count(), s.next_seq++),
                OrderBookL3::Action::kCancel);
  book.onRecord(makeRec(s.ask_id, OrderBookL3::Side::kAsk, ba->price, 0, now.count(), s.next_seq++),
                OrderBookL3::Action::kCancel);

  // New IDs based on current time for determinism & uniqueness.
  s.bid_id = "B" + std::to_string(now.count());
  s.ask_id = "A" + std::to_string(now.count());

  // Add the new top-of-book quotes.
  book.onRecord(makeRec(s.bid_id, OrderBookL3::Side::kBid, new_bid, 1'000, now.count(), s.next_seq++),
                OrderBookL3::Action::kAdd);
  book.onRecord(makeRec(s.ask_id, OrderBookL3::Side::kAsk, new_ask, 1'000, now.count(), s.next_seq++),
                OrderBookL3::Action::kAdd);
}

}  // namespace

int main() {
  try {
    // ---- Config ----
    const std::string ticker = "GAZP";
    const std::string out_dir = "out_s6_demo";

    // ---- L3 book seeded with quotes ----
    libbacktester::OrderBookL3 book(ticker);
    auto seed = seedBook(book);

    // ---- Strategy: Buy 1000 over 60s in 10 equal slices ----
    // ctor order: (strategy_id, side, total_qty, start_ts, end_ts, num_slices)
    libbacktester::TWAPStrategy twap("twap_demo",
                                     OrderSide::kBuy,
                                     /*total_qty=*/1000,
                                     /*start_ts=*/Ms{0},
                                     /*end_ts=*/Ms{60'000},
                                     /*num_slices=*/10);

    // ---- Portfolio / Metrics / Executor wiring ----
    libbacktester::Portfolio portfolio;
    libbacktester::MetricsWriter::init(out_dir);
    libbacktester::ExecutionSimulator exec(&book, &twap, &portfolio);

    // ---- Replay loop (1s ticks for 60s) ----
    for (Ms now{0}; now <= Ms{60'000}; now += Ms{1'000}) {
      // Move quotes every 10s to make MTM non-trivial.
      if (now.count() != 0 && (now.count() % 10'000 == 0)) {
        stepQuotes(book, seed, now);
      }

      // Drive strategy timer -> generate intents -> execute.
      twap.onTimer(now);
      auto intents = twap.popPendingIntents();
      exec.submitIntents(std::move(intents), now);

      // Mark-to-market at mid if both sides exist.
      const auto bb = book.bestBid();
      const auto ba = book.bestAsk();
      const double bid = bb ? bb->price : 0.0;
      const double ask = ba ? ba->price : 0.0;
      portfolio.markToMarket(bid, ask);

      // Persist equity snapshot.
      libbacktester::MetricsWriter::instance().recordEquity(now, portfolio.equity());
    }

    // ---- Final console summary ----
    const auto bb = book.bestBid();
    const auto ba = book.bestAsk();
    std::cout << "S6 demo finished.\n"
              << "Output CSVs: " << out_dir << "\n"
              << "Final position: " << portfolio.position() << "\n"
              << "Final cash: " << portfolio.cash() << "\n"
              << "Realized PnL: " << portfolio.realizedPnl() << "\n"
              << "Mark price: " << portfolio.markPrice() << "\n"
              << "Equity: " << portfolio.equity() << "\n"
              << "Book top: bid="
              << (bb ? std::to_string(bb->price) : "n/a")
              << " / ask="
              << (ba ? std::to_string(ba->price) : "n/a")
              << "\n";

    return 0;
  } catch (const std::exception& ex) {
    std::cerr << "Fatal: " << ex.what() << "\n";
    return 1;
  }
}
