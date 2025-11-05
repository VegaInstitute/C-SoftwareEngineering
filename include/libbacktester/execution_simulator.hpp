#ifndef LIBBACKTESTER_EXECUTION_SIMULATOR_HPP_
#define LIBBACKTESTER_EXECUTION_SIMULATOR_HPP_

#include <optional>
#include <vector>

#include "libbacktester/orderbook.hpp"  // OrderBookL3 public API
#include "libbacktester/strategy.hpp"   // Strategy base
#include "libbacktester/types.hpp"      // Ms, OrderIntent, FillEvent, OrderSide

namespace libbacktester {

class Portfolio;      // fwd
class MetricsWriter;  // fwd

/* @class ExecutionSimulator
 *
 * @brief Deterministic top-of-book execution simulator (single-threaded, C++20).
 *
 * Business rules:
 *  - Buy fills at best ask price.
 *  - Sell fills at best bid price.
 *  - Missing required side → skip.
 *  - Non-positive qty → skip.
 *  - Batch processed in supplied order; fill timestamp equals submitIntents() ts.
 *
 * Notifications on a successful fill:
 *  1) Strategy::onFill(fill)
 *  2) Portfolio::onFill(fill)
 *  3) MetricsWriter::recordTrade(fill)
 */
class ExecutionSimulator {
 public:
  /**
   * @brief Construct simulator bound to book/strategy/portfolio.
   * @param book Non-owning; must outlive this object.
   * @param strategy Non-owning; must outlive this object.
   * @param portfolio Non-owning; must outlive this object.
   */
  ExecutionSimulator(OrderBookL3* book,
                     Strategy* strategy,
                     Portfolio* portfolio) noexcept;

  /**
   * @brief Consume and price a batch of intents at time ts.
   * @param intents Newly-due intents (rvalue to avoid copy).
   * @param ts Simulation timestamp in milliseconds.
   */
  void submitIntents(std::vector<OrderIntent>&& intents, Ms ts);

 private:
  OrderBookL3* book_;      // non-owning
  Strategy* strategy_;     // non-owning
  Portfolio* portfolio_;   // non-owning

  [[nodiscard]] static std::optional<double> topPriceForSide(
      const OrderBookL3* book, OrderSide side) noexcept;

  void emitFillAndNotify(const OrderIntent& intent, Ms ts, double px);
};

}  // namespace libbacktester

#endif  // LIBBACKTESTER_EXECUTION_SIMULATOR_HPP_
