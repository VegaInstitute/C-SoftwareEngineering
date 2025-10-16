#include "libbacktester/portfolio.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>

namespace libbacktester {

void Portfolio::applyCash(const FillEvent& fill) noexcept {
  if (fill.qty <= 0) return;
  const double notional = static_cast<double>(fill.qty) * fill.price;
  if (fill.side == OrderSide::kBuy) {
    cash_ -= notional;
  } else {
    cash_ += notional;
  }
}

void Portfolio::realizeIfClosing(const FillEvent& fill) noexcept {
  if (fill.qty <= 0) return;

  const bool reducing_long = (position_ > 0 && fill.side == OrderSide::kSell);
  const bool reducing_short = (position_ < 0 && fill.side == OrderSide::kBuy);
  if (!reducing_long && !reducing_short) return;

  const std::int64_t qty_close =
      std::min<std::int64_t>(std::llabs(position_), fill.qty);
  if (qty_close <= 0) return;

  const double pnl_per_unit =
      (position_ > 0) ? (fill.price - avg_price_)   // closing long
                      : (avg_price_ - fill.price);  // covering short

  realized_pnl_ += static_cast<double>(qty_close) * pnl_per_unit;
}

void Portfolio::onFill(const FillEvent& fill) noexcept {
  if (fill.qty <= 0) return;

  applyCash(fill);
  realizeIfClosing(fill);

  if (fill.side == OrderSide::kBuy) {
    const std::int64_t new_pos = position_ + fill.qty;

    if (position_ >= 0) {
      const double notional_old = static_cast<double>(position_) * avg_price_;
      const double notional_new = static_cast<double>(fill.qty) * fill.price;
      const std::int64_t total = position_ + fill.qty;
      avg_price_ = (total > 0)
                       ? (notional_old + notional_new) /
                             static_cast<double>(total)
                       : 0.0;
    } else {
      if (new_pos > 0) {
        avg_price_ = fill.price;
      } else if (new_pos == 0) {
        avg_price_ = 0.0;
      }
    }
    position_ = new_pos;

  } else {  // Sell
    const std::int64_t new_pos = position_ - fill.qty;

    if (position_ <= 0) {
      const double notional_old = static_cast<double>(-position_) * avg_price_;
      const double notional_new = static_cast<double>(fill.qty) * fill.price;
      const std::int64_t total = (-position_) + fill.qty;
      avg_price_ = (total > 0)
                       ? (notional_old + notional_new) /
                             static_cast<double>(total)
                       : 0.0;
    } else {
      if (new_pos < 0) {
        avg_price_ = fill.price;
      } else if (new_pos == 0) {
        avg_price_ = 0.0;
      }
    }
    position_ = new_pos;
  }
}

void Portfolio::markToMarket(double bid, double ask) noexcept {
  if (bid > 0.0 && ask > 0.0) {
    mark_price_ = 0.5 * (bid + ask);
  }
}

}  // namespace libbacktester
