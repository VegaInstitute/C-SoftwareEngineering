#ifndef LIBBACKTESTER_STRATEGY_FACTORY_HPP_
#define LIBBACKTESTER_STRATEGY_FACTORY_HPP_

#include <cstdlib>
#include <memory>
#include <string>
#include "libbacktester/strategy.hpp"
#include "libbacktester/types.hpp"

namespace libbacktester {
  struct TWAPSWIGParams {
    std::string strategy_id;
    OrderSide side;
    std::int64_t total_qty;
    Ms start_ts;
    Ms end_ts;
    std::size_t slices;
    bool force_all_equal;
  };

  class StrategyFactory {
   public:
    /* @brief A TWAP factory pattern.
     * @param params
     * @throws std::invalid_argument
     */
    static std::unique_ptr<Strategy> createTWAP(const TWAPSWIGParams& params);
  };
}

#endif // LIBBACKTESTER_STRATEGY_FACTORY_HPP_
