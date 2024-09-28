#include <hft_cpp/hft_data_loader.h>

#include <iostream>
#include <fstream>
#include <sstream>

void OrderBook::print() {
    std::cout << "Order book at " << time << " with best ask " << ask.price << " and best bid " << bid.price << std::endl;
}

void OrderBook::update(std::string new_time, double new_ask_price, double new_ask_volume, double new_bid_price, double new_bid_volume) {
    time = new_time;

    ask.price = new_ask_price;
    ask.volume = new_ask_volume;

    bid.price = new_bid_price;
    bid.volume = new_bid_volume;
}

bool OrderBookLoader::iterate_and_print() {
    std::string line;
    std::string time;
    double values[4];

    auto file = std::ifstream(file_path);

    while (std::getline(file, line)) {
        std::stringstream strstr(line);
        std::getline(strstr, time, ',');

        std::string temp;
        for (int i = 0; i < 4; i++) {
            std::getline(strstr, temp, ',');
            values[i] = std::stod(temp);
        }

        order_book.update(time, values[0], values[2], values[1], values[3]);
        order_book.print();
    }

    return true;
}


OrderBook OrderBookLoader::get_order_book_state() {
    return order_book;
}
