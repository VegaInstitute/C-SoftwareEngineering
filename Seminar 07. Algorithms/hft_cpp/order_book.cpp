#include <hft_cpp.hpp>
#include <iostream>
#include <algorithm>

void OrderBook::add_order(double price, double volume, bool is_buy) {
    LimitOrder order {++order_id_counter, price, volume, is_buy};
    int price_level_number = int(price/price_level_step);
    if (is_buy) {
        bid[price_level_number].push_back(order); // FIFO
    } else {
        ask[price_level_number].push_back(order); // FIFO
    }

    std::cout << "Added limit order @ price level #" << price_level_number << " with id " << order.id << " and volume " << order.volume << std::endl;
}

void OrderBook::execute_market_order(MarketOrder market_order) {
    if (market_order.is_buy) {
        double matched_volume = 0.;

        while (market_order.volume > 0 && !ask.empty()) {
            auto iter = ask.begin();
            auto price_level_idx = iter->first;
            auto price_level = iter->second;

            for (auto level_iter = price_level.begin(); level_iter != price_level.end();) {
                LimitOrder limit_order = *level_iter;
                double trade_volume = std::min(limit_order.volume, market_order.volume);
                matched_volume += trade_volume;
                market_order.volume -= trade_volume;

                // assume that no fractional amounts can be placed in the order book
                if (limit_order.volume <= trade_volume) {
                    price_level.erase(level_iter);
                }
                else {
                    limit_order.volume -= trade_volume;
                }

                if (market_order.volume <= 0.) { break; }
                ++level_iter;
            }

            if (price_level.empty()) {
                ask.erase(iter);
            }
        }
    } else {
        throw NotImplemented();
    }
}
