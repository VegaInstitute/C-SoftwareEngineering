#ifndef LIBBACKTESTER_TYPES_HPP_
#define LIBBACKTESTER_TYPES_HPP_

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

namespace libbacktester {

// All public timestamps use std::chrono::milliseconds (ms since a shared epoch).
using Ms = std::chrono::milliseconds;

// Shared side enum for intents/fills.
enum class OrderSide : std::uint8_t { kBuy = 0, kSell = 1 };

//------------------------------------------------------------------------------
// Lightweight engine/strategy surface types
//------------------------------------------------------------------------------

// Minimal book/event tick surfaced to strategies for determinism.
// (S5 strategies may ignore it; we still define it for API stability.)
struct BookEvent {
  Ms ts{Ms{0}};             // event timestamp (ms)
  std::uint64_t seq{0};     // monotone engine/event sequence number
};

// Execution/fill notification for a child order belonging to a strategy.
// (Defined for S6+; present here to keep the Strategy interface stable.)
struct FillEvent {
  std::string strategy_id;      // who owns the child order
  std::string order_local_id;   // strategy-scoped id (e.g., "twap-3")
  OrderSide side{OrderSide::kBuy};
  double price{0.0};            // execution price
  std::int64_t qty{0};          // executed quantity (>0)
  Ms ts{Ms{0}};                 // fill timestamp
};

// Intent to submit an order (engine decides when/how to route; S5 only emits).
// Price is optional (std::nullopt means "no limit"/price discovery in S5).
struct OrderIntent {
  std::string strategy_id;      // non-empty
  std::string local_id;         // deterministic per-strategy (e.g., "<strategy>-<i>")
  OrderSide side{OrderSide::kBuy};
  std::optional<double> price;  // limit price if present; std::nullopt in S5
  std::int64_t qty{0};          // strictly > 0
  Ms submit_ts{Ms{0}};          // when this intent becomes due for submission
};

}  // namespace libbacktester

#endif  // LIBBACKTESTER_TYPES_HPP_
