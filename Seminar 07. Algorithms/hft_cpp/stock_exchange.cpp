#include <hft_cpp.hpp>
#include <iostream>
#include <optional>
#include <regex>
#include <string>

std::optional<Command> parse_to_command(const std::string &input) {
    std::regex pattern(R"(^(BUY|SELL|REMOVE)\s+([A-Z]+)\s+(\d+(\.\d+)?)\s*(\d+(\.\d+)?)?$)");
    std::smatch matches;

    if (std::regex_match(input, matches, pattern)) {
        if (matches.size() >= 4) {
            Command command {
                matches[1],
                matches[2],
                std::stod(matches[3]),
                matches[5].matched ? std::make_optional(std::stod(matches[5])) : std::nullopt
            };
            return std::make_optional(command);
        }
    }
    return std::nullopt;
}

void StockExchange::process_command(const Command& command) {
    OrderBook &order_book = order_books[command.ticker];

    std::cout << "Processing " << command.ticker << " " << command.action << std::endl;

    if (command.action == "BUY") {
        if (command.price.has_value()) {
            std::cout << "Adding " << command.ticker << " " << command.action << std::endl;
            order_book.add_order(command.price.value(), command.volume, true);
        } else {
            order_book.execute_market_order(MarketOrder {command.volume, true});
        }
    } else if (command.action == "SELL") {
        if (command.price.has_value()) {
            order_book.add_order(command.price.value(), command.volume, false);
        } else {
            order_book.execute_market_order(MarketOrder {command.volume, false});
        }
    } else {
        std::cout << "Action type not implemented." << std::endl;
    }
}
