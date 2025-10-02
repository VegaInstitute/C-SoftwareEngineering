#include "libbacktester/orderbook.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <SECCODE> <csvfile>\n";
    return 1;
  }
  std::string ticker = argv[1];
  std::string csvfile = argv[2];

  backtester::OrderBookL3 ob(ticker);

  try {
    ob.LoadCsv(csvfile);
  } catch (const std::exception& ex) {
    std::cerr << "Error loading CSV: " << ex.what() << "\n";
    return 1;
  }

  auto b = ob.bestBid();
  auto a = ob.bestAsk();
  if (b) {
    std::cout << "Best Bid: " << b->price << " × " << b->agg_qty << "\n";
  } else {
    std::cout << "No bids\n";
  }
  if (a) {
    std::cout << "Best Ask: " << a->price << " × " << a->agg_qty << "\n";
  } else {
    std::cout << "No asks\n";
  }

  std::cout << "Bid side levels:\n";
  ob.ForEachLevel(backtester::OrderBookL3::Side::kBid,
                  [&](const backtester::OrderBookL3::Level& lvl) {
    std::cout << "  " << lvl.price << " → " << lvl.agg_qty
              << " (count=" << lvl.count << ")\n";
  });
  std::cout << "Ask side levels:\n";
  ob.ForEachLevel(backtester::OrderBookL3::Side::kAsk,
                  [&](const backtester::OrderBookL3::Level& lvl) {
    std::cout << "  " << lvl.price << " → " << lvl.agg_qty
              << " (count=" << lvl.count << ")\n";
  });

  return 0;
}
