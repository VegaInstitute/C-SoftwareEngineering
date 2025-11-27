#include "libbacktester/strategies/strategy_factory.hpp"
#include "libbacktester/strategies/twap_strategy.hpp"
#include <memory>
#include <stdexcept>


namespace libbacktester {
  // TODO parameter validation
  [[nodiscard]] bool validateTWAPParams(const TWAPSWIGParams& _) {
      return true;
  }


  std::unique_ptr<libbacktester::Strategy> StrategyFactory::createTWAP(const TWAPSWIGParams& params) {
    if (validateTWAPParams(params))
    {
        return std::make_unique<TWAPStrategy>(
            TWAPStrategy(
                params.strategy_id,
                params.side,
                params.total_qty,
                params.start_ts,
                params.end_ts,
                params.slices,
                params.force_all_equal
            )
        );
    } else {
        throw std::invalid_argument("TWAP parameters did not pass the validation layer. Please, check the inputs.");
    }
  }
}
