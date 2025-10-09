#ifndef LIBBACKTESTER_STRATEGY_HPP_
#define LIBBACKTESTER_STRATEGY_HPP_

#include <chrono>
#include <cstdint>
#include <vector>
#include "libbacktester/types.hpp"

namespace libbacktester {

/**
 * @class Strategy
 * @brief Base interface for all strategies in the single-threaded, deterministic engine.
 *
 * @details
 * **Time semantics**
 *   - All public timestamps use `std::chrono::milliseconds` (alias: `Ms`) to make units explicit. \n
 *   - Given the same call sequence, a Strategy must behave deterministically (pure function of config + calls).
 *
 * **Hot path (S5 constraints)**
 *   - `onTimer()` MUST NOT allocate and MUST NOT throw. \n
 *   - Emission happens via `popPendingIntents()`, which may allocate to form the return vector.
 *
 * **Emission semantics**
 *   - `onTimer(ts)` marks all slices/intents with `due <= ts` as **due** (idempotent for repeated `ts`). \n
 *   - `popPendingIntents()` returns **all** newly-due intents since the last pop, in non-decreasing `submit_ts`,
 *     stable for ties, and then **drains** them (no double-emit).
 *
 * @note Use of `std::chrono` is recommended for clarity and type safety with time values.  [oai_citation:2‡Cppreference](https://en.cppreference.com/w/cpp/chrono/duration.html?utm_source=chatgpt.com)
 */
class Strategy {
 public:
  using Ms = std::chrono::milliseconds;

  /// Defaulted base constructor (required when deriving with custom ctors).
  Strategy() = default;

  /// Virtual destructor.
  virtual ~Strategy() = default;

  Strategy(const Strategy&) = delete;
  Strategy& operator=(const Strategy&) = delete;
  Strategy(Strategy&&) = default;
  Strategy& operator=(Strategy&&) = default;

  /**
   * @brief Notify the strategy that the order book advanced.
   * @param ts   Event timestamp (ms).
   * @param seq  Monotone engine/event sequence number.
   * @note TWAP ignores book updates in S5; hook reserved for future features.
   * @warning Must not throw and should not allocate.
   */
  virtual void onBookUpdate(Ms ts, std::uint64_t seq) noexcept = 0;

  /**
   * @brief Deterministic timer hook.
   * @param ts Current engine time (ms).
   * @details Advances the internal cursor so all intents with `due <= ts` become due.
   *          Multiple calls with the same `ts` are idempotent.
   * @warning MUST NOT allocate and MUST NOT throw.
   */
  virtual void onTimer(Ms ts) noexcept = 0;

  /**
   * @brief Report fills for this strategy's child orders.
   * @param fill Fill details.
   * @note Not used in S5; defined now for API stability (wired in S6+).
   */
  virtual void onFill(const FillEvent& fill) = 0;

  /**
   * @brief Return and clear all newly-due intents since the last pop.
   * @return Intents in non-decreasing `submit_ts` order; stable for ties.
   * @note This function may allocate to compose the return vector. Never double-emits.
   */
  virtual std::vector<OrderIntent> popPendingIntents() = 0;
};

}  // namespace libbacktester

#endif  // LIBBACKTESTER_STRATEGY_HPP_
