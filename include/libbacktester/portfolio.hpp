#ifndef LIBBACKTESTER_PORTFOLIO_HPP_
#define LIBBACKTESTER_PORTFOLIO_HPP_

#include <cstdint>

#include "libbacktester/types.hpp"  // Ms, FillEvent, OrderSide

namespace libbacktester {

/**
 * @file portfolio.hpp
 * @brief Average-price portfolio accounting with mid-price MTM (deterministic).
 *
 * Rules:
 *  - Cash: Buy decreases; Sell increases by qty*price.
 *  - Realized P&L only on reductions:
 *      * Close long:   qty_closed * (fill - avg).
 *      * Cover short:  qty_closed * (avg - fill).
 *  - Increasing exposure on same side → VWAP update of avg_price_.
 *  - Reducing exposure without sign flip → avg_price_ unchanged.
 *  - Crossing zero → avg_price_ = entry price of new exposure.
 *  - markToMarket(bid, ask) sets mid if both > 0; otherwise unchanged.
 *  - equity() = cash_ + position_ * mark_price_.
 *  - Zero-qty fill is ignored.
 */

 /**
  * @class Portfolio
  * @brief Manages cash, position, P\&L, and mark-to-market for executed fills.
  *
  * @details
  * The portfolio uses an average-cost accounting model:
  * - Buys reduce cash, sells increase cash.
  * - Realized P\&L is recognized when reducing an existing position (closing or covering).
  * - On side additions (same side), the average price (basis) is updated via VWAP.
  * - Crossing zero resets the basis to the new side’s fill price.
  * - markToMarket(bid, ask) sets the mark price to the midpoint if both are > 0.
  * - equity() = cash + position * mark_price.
  * - Zero-quantity fills are ignored (no effect).
  */
class Portfolio {
 public:
  Portfolio() noexcept = default;

  /**
   * @brief Applies a fill event to update cash, position, and realized P\&L.
   * @param fill The fill event (qty, price, side, timestamp).
   *
   * If fill.qty <= 0, this is a no-op.
   */
  void onFill(const FillEvent& fill) noexcept;

  /**
   * @brief Marks the portfolio to market using bid/ask mid.
   * @param bid Best bid price (must be > 0 to use).
   * @param ask Best ask price (must be > 0 to use).
   *
   * Only updates mark_price_ if both bid and ask are positive.
   */
  void markToMarket(double bid, double ask) noexcept;

  /**
   * @brief Computes current equity = cash + position * mark_price.
   * @return The current equity.
   */
  [[nodiscard]] double equity() const noexcept {
    return cash_ + static_cast<double>(position_) * mark_price_;
  }

  /// @name Accessors
  [[nodiscard]] double cash() const noexcept { return cash_; }
  [[nodiscard]] std::int64_t position() const noexcept { return position_; }
  [[nodiscard]] double realizedPnl() const noexcept { return realized_pnl_; }
  [[nodiscard]] double markPrice() const noexcept { return mark_price_; }
  [[nodiscard]] double avgPrice() const noexcept { return avg_price_; }
  /// @}


 private:
  double cash_{0.0};
  std::int64_t position_{0};
  double realized_pnl_{0.0};
  double mark_price_{0.0};
  double avg_price_{0.0};

  void applyCash(const FillEvent& fill) noexcept;
  void realizeIfClosing(const FillEvent& fill) noexcept;
};

}  // namespace libbacktester

#endif  // LIBBACKTESTER_PORTFOLIO_HPP_
