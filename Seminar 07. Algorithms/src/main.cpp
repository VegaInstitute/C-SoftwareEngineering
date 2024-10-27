#include <hft_cpp.hpp>
#include <iostream>
#include <vector>

int main() {
    StockExchange se;

    std::vector<std::string> commands = {
        "BUY VTBR 100 0.02",
        "BUY VTBR 120 0.02",
        "BUY VTBR 110 0.025",
    };

    for (const auto &command : commands) {
        auto parsed_command = parse_to_command(command);
        if (parsed_command) {
            Command cmd = parsed_command.value();
            se.process_command(parsed_command.value());
        } else {
            std::cout << "Failed to parse the following command:\n" << command << "\n\n" << std::endl;
        }
    }

    return 0;
}
