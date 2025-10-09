#ifndef LIBBACKTESTER_STRATEGIES_TWAP_STRATEGY_HPP_
#define LIBBACKTESTER_STRATEGIES_TWAP_STRATEGY_HPP_

#include <cstddef>
#include <string>
#include <vector>

#include "libbacktester/types.hpp"    // Ms, OrderIntent, FillEvent, OrderSide
#include "libbacktester/strategy.hpp" // Strategy base

namespace libbacktester {

/**
 * @class TWAPStrategy
 * @brief Time-sliced execution strategy that emits precomputed order intents on a deterministic schedule.
 *
 * @details
 * **Purpose.** Execute a parent order in evenly timed slices to reduce market impact and achieve a
 * time-weighted average execution price over a fixed window.
 *
 * **Schedule (time grid).**
 * Let `N` be the number of slices and `span = end - start` (milliseconds). For slice index `i`:
 * @code
 *   due_i = start + floor(i * span / N)     for i = 1..N-1
 *   due_N = end
 * @endcode
 * The grid is monotone non-decreasing; ties occur when `span < N`. Ties are emitted in slice-index order.
 *
 * **Quantity split (sum preserved).**
 * With `base = total / N` and `rem = total % N`:
 *  - Default (prefix remainder): slices `1..rem` get `base+1`, the rest get `base`.
 *  - `force_all_equal = true`: slices `1..N-1` get `base`, slice `N` gets `total - base*(N-1)`.
 * Slices with `qty <= 0` are skipped (no intents emitted for zero quantity).
 *
 * **Determinism & performance.**
 * All (non-zero) `OrderIntent`s are materialized in the constructor. The hot path (`onTimer`) advances
 * a cursor only—**no dynamic allocations**, idempotent for repeated timestamps. `popPendingIntents()`
 * drains newly-due intents without double-emitting.
 */
class TWAPStrategy final : public Strategy {
 public:
  /**
   * @brief Construct a TWAP strategy with explicit parameters.
   * @param strategy_id     Non-empty identifier; also used to form deterministic `local_id`s (e.g., "<id>-<i>").
   * @param side            Parent order side.
   * @param total_qty       Parent order quantity (> 0).
   * @param start_ts        Start time (inclusive).
   * @param end_ts          End time (>= last due; must be > start_ts).
   * @param slices          Number of time slices (> 0).
   * @param force_all_equal If true, last slice collects all remainder; otherwise remainder is prefix-distributed.
   * @throws std::invalid_argument if any invariant is violated.
   *
   * @invariant `end_ts > start_ts`, `slices >= 1`, `total_qty > 0`, `strategy_id` non-empty.
   * @invariant Materialized intents are in non-decreasing `submit_ts` and stable for ties.
   * @invariant `onTimer()` is zero-alloc and idempotent for repeated `ts`.
   */
  TWAPStrategy(std::string strategy_id,
               OrderSide side,
               std::int64_t total_qty,
               Ms start_ts,
               Ms end_ts,
               std::size_t slices,
               bool force_all_equal = false);

  /// @copydoc Strategy::onBookUpdate
  void onBookUpdate(Ms ts, std::uint64_t seq) noexcept override;

  /// @copydoc Strategy::onTimer
  void onTimer(Ms ts) noexcept override;

  /// @copydoc Strategy::onFill
  void onFill(const FillEvent& fill) override;

  /// @copydoc Strategy::popPendingIntents
  std::vector<OrderIntent> popPendingIntents() override;

  // ----------------------- Introspection (tests/metrics) -----------------------

  /// @brief Strategy identifier.
  const std::string& strategyId() const noexcept { return strategy_id_; }
  /// @brief Parent order side.
  OrderSide side() const noexcept { return side_; }
  /// @brief Parent order total quantity.
  std::int64_t totalQty() const noexcept { return total_qty_; }
  /// @brief Start time.
  Ms startTs() const noexcept { return start_ts_; }
  /// @brief End time.
  Ms endTs() const noexcept { return end_ts_; }
  /// @brief Number of requested slices.
  std::size_t slices() const noexcept { return slices_; }
  /// @brief Remainder policy flag.
  bool forceAllEqual() const noexcept { return force_all_equal_; }

  /// @brief Number of non-zero intents materialized in the constructor.
  std::size_t totalIntentsMaterialized() const noexcept { return intents_.size(); }
  /// @brief Index of first intent not yet due at the last `onTimer()` call.
  std::size_t nextDueIndex() const noexcept { return next_due_index_; }
  /// @brief Index of first intent not yet returned by `popPendingIntents()`.
  std::size_t lastPoppedIndex() const noexcept { return last_popped_index_; }

 private:
  /**
   * @brief Precompute schedule and materialize non-zero intents in due-time order.
   * @details Complexity: O(N) time and up to O(N) storage (less if zero-qty slices are skipped).
   * Uses integer-only arithmetic for offsets; ties remain in original slice order.
   */
  void buildScheduleAndIntents();

  // Validated ctor arguments
  std::string  strategy_id_;
  OrderSide    side_{OrderSide::kBuy};
  std::int64_t total_qty_{0};
  Ms           start_ts_{Ms{0}};
  Ms           end_ts_{Ms{0}};
  std::size_t  slices_{0};
  bool         force_all_equal_{false};

  // Precomputed immutable intents (moved out during pop).
  std::vector<OrderIntent> intents_;

  // Monotone indices
  std::size_t next_due_index_{0};
  std::size_t last_popped_index_{0};
};

}  // namespace libbacktester

#endif  // LIBBACKTESTER_STRATEGIES_TWAP_STRATEGY_HPP_
