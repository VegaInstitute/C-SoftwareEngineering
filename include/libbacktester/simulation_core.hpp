#ifndef LIBBACKTESTER_SIMULATION_CORE_HPP_
#define LIBBACKTESTER_SIMULATION_CORE_HPP_

#include <string>
#include "libbacktester/orderbook.hpp"
#include "libbacktester/strategy.hpp"
#include "libbacktester/execution_simulator.hpp"
#include "libbacktester/portfolio.hpp"
#include "libbacktester/metrics_writer.hpp"
#include "libbacktester/types.hpp"

namespace libbacktester {

/**
 * @brief Deterministic single-threaded orchestrator of the backtest.
 *
 * Responsibilities:
 *  - Replay MOEX Full Orders Log CSV for the book's ticker.
 *  - Maintain a simulated clock in milliseconds (Ms).
 *  - Fire periodic timer ticks with strict ordering vs. market events:
 *      (1) market events at ts, (2) onTimer(ts), (3) submit intents,
 *      (4) mark-to-market & record equity at ts.
 *  - Produce trades.csv (via ExecutionSimulator/MetricsWriter) and equity.csv.
 *
 * Non-goals in this project: latency/fees/slippage, multithreading.
 */
class SimulationCore {
 public:
  SimulationCore(OrderBookL3* book,
                 Strategy* strategy,
                 ExecutionSimulator* exec,
                 Portfolio* portfolio,
                 MetricsWriter* metrics) noexcept;

  /**
   * @brief Run the backtest end-to-end.
   * @param csv_path   Path to MOEX Full Orders Log CSV.
   * @param timer_step Fixed timer step (e.g., Ms{1000} for 1s ticks).
   *
   * Deterministic ordering rule when event_ts == timer_ts:
   *   1) process all market events at ts,
   *   2) fire onTimer(ts),
   *   3) drain popPendingIntents() and submit to ExecutionSimulator,
   *   4) mark-to-market & record equity at ts.
   */
  void run(const std::string& csv_path, Ms timer_step);

 private:
  // Helpers (implementation detail; not part of public API).
  void fireTimerTick_(Ms ts);
  static Ms parseMoexTimeMs_(const std::string& hhmmsszzz_or_hhmmsszzzxxx);
  static OrderBookL3::Action toAction_(int action);
  static OrderBookL3::Side   toSide_(char c);

  OrderBookL3*        book_;
  Strategy*           strategy_;
  ExecutionSimulator* exec_;
  Portfolio*          portfolio_;
  MetricsWriter*      metrics_;

  Ms now_{Ms{0}};
  Ms next_timer_ts_{Ms{0}};
};

}  // namespace libbacktester

#endif  // LIBBACKTESTER_SIMULATION_CORE_HPP_
