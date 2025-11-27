#ifndef LIBBACKTESTER_PYTHON_BINDINGS_HPP_
#define LIBBACKTESTER_PYTHON_BINDINGS_HPP_

#include <string>
#include "libbacktester/strategies/strategy_factory.hpp"
#include "libbacktester/types.hpp"

namespace libbacktester {
    struct RunSpec {
        std::string data_csv_path;
        std::string instrument;
        std::string out_path;
        Ms tick_ms;
        std::string strategy_name;
        TWAPSWIGParams strategy_params;
    };

    struct RunResult {
        int result_code{0};
        std::string out_path;
        std::string trades_csv_path;
        std::string equity_csv_path;
        std::string top_of_the_book_csv_path;
        std::string log;
    };

    class Strategy; // fwd

    RunResult run_backtest(const RunSpec& spec, Strategy* strategy);
    RunResult run_backtest(const RunSpec& runSpec);
}

#endif // LIBBACKTESTER_PYTHON_BINDINGS_HPP_
