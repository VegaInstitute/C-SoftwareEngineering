#include "libbacktester/execution_simulator.hpp"

#include <cassert>

#include "libbacktester/metrics_writer.hpp"
#include "libbacktester/portfolio.hpp"

namespace libbacktester {

ExecutionSimulator::ExecutionSimulator(OrderBookL3* book,
                                       Strategy* strategy,
                                       Portfolio* portfolio) noexcept
    : book_(book), strategy_(strategy), portfolio_(portfolio) {
  assert(book_ != nullptr && strategy_ != nullptr && portfolio_ != nullptr);
}

std::optional<double> ExecutionSimulator::topPriceForSide(
    const OrderBookL3* book, OrderSide side) noexcept {
  if (side == OrderSide::kBuy) {
    const auto ask = book->bestAsk();
    if (ask.has_value()) return ask->price;
    return std::nullopt;
  } else {
    const auto bid = book->bestBid();
    if (bid.has_value()) return bid->price;
    return std::nullopt;
  }
}

void ExecutionSimulator::emitFillAndNotify(const OrderIntent& intent,
                                           Ms ts,
                                           double px) {
  FillEvent fill{};
  fill.strategy_id = intent.strategy_id;
  fill.side = intent.side;
  fill.qty = intent.qty;
  fill.price = px;
  fill.ts = ts;

  strategy_->onFill(fill);
  portfolio_->onFill(fill);
  MetricsWriter::instance().recordTrade(fill);  // <- instance()
}

void ExecutionSimulator::submitIntents(std::vector<OrderIntent>&& intents,
                                       Ms ts) {
  for (const auto& oi : intents) {
    if (oi.qty <= 0) continue;
    const auto px = topPriceForSide(book_, oi.side);
    if (!px.has_value()) continue;
    emitFillAndNotify(oi, ts, *px);
  }
}

}  // namespace libbacktester
