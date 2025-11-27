#include "libbacktester/bindings.hpp"

#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <string>

#include "libbacktester/execution_simulator.hpp"
#include "libbacktester/metrics_writer.hpp"
#include "libbacktester/orderbook.hpp"
#include "libbacktester/portfolio.hpp"
#include "libbacktester/simulation_core.hpp"
#include "libbacktester/strategy.hpp"
#include "libbacktester/strategies/strategy_factory.hpp"
#include "libbacktester/types.hpp"

namespace fs = std::filesystem;

namespace libbacktester {
namespace {
    std::string Join(const fs::path& a, const std::string& b) {
        return (a / b).string();
    }

    bool FileExists(const std::string& p) {
        std::error_code ec;
        return fs::is_regular_file(fs::path(p), ec);
    }

    fs::path EnsureOutdir(const std::string& p) {
        fs::path out = p.empty() ? fs::path("out") : fs::path(p);
        std::error_code ec;
        fs::create_directories(out, ec);  // idempotent
        return out;
    }

    bool IEquals(std::string a, std::string b) {
        std::transform(a.begin(), a.end(), a.begin(), ::tolower);
        std::transform(b.begin(), b.end(), b.begin(), ::tolower);
        return a == b;
    }
}  // namespace


RunResult run_backtest(const RunSpec& spec, Strategy* strategy) {
  if (!FileExists(spec.data_csv_path)) {
    throw std::runtime_error("data_csv not found: " + spec.data_csv_path);
  }
  if (spec.instrument.empty()) throw std::runtime_error("instrument must be non-empty");
  if (spec.tick_ms.count() <= 0) throw std::runtime_error("timer_step_ms must be > 0");

  const auto outdir = EnsureOutdir(spec.out_path);

  OrderBookL3   book(spec.instrument);
  Portfolio     portfolio;
  MetricsWriter::init(spec.out_path);
  MetricsWriter &metrics = MetricsWriter::instance();

  ExecutionSimulator exec(&book, strategy, &portfolio);
  SimulationCore     core(&book, strategy, &exec, &portfolio, &metrics);

  core.run(spec.data_csv_path, Ms{spec.tick_ms});

  RunResult rr;
  rr.result_code            = 0;
  rr.out_path          = outdir.string();
  rr.trades_csv_path      = Join(outdir, "trades.csv");
  rr.equity_csv_path      = Join(outdir, "equity.csv");
  rr.top_of_the_book_csv_path = Join(outdir, "top_of_book.csv");
  return rr;
}


RunResult run_backtest(const RunSpec& spec) {
  std::unique_ptr<Strategy> strategy;
  if (IEquals(spec.strategy_name, "TWAP")) {
    strategy = StrategyFactory::createTWAP(spec.strategy_params);
  } else {
    throw std::runtime_error("unknown strategy_name: " + spec.strategy_name);
  }
  return run_backtest(spec, strategy.get());
}

}  // namespace libbacktester
