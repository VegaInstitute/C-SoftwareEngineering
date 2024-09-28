#ifndef HFT_DATA_LOADER_H
#define HFT_DATA_LOADER_H

#include <string>

class OrderBookSide {
    public:
        double price;
        double volume;

        OrderBookSide(double price, double volume) : price(price), volume(volume) {}
        friend class OrderBook;
        friend class OrderBookLoader;
};

class OrderBook {
    public:
        std::string time;
        OrderBookSide ask;
        OrderBookSide bid;

        OrderBook(std::string time, OrderBookSide ask, OrderBookSide bid) : time(time), ask(ask), bid(bid) {}
        friend class OrderBookLoader;

        void print();
        void update(std::string new_time, double new_ask_price, double new_ask_volume, double new_bid_price, double new_bid_volume);
};

class OrderBookLoader {
    public:
        std::string file_path;
        OrderBook order_book;

        OrderBookLoader(std::string path) : file_path(path), order_book(OrderBook(std::string(""), OrderBookSide(0., 0.), OrderBookSide(0., 0.))) {}

        OrderBook get_order_book_state();
        bool iterate_and_print();
};

#endif // HFT_DATA_LOADER_H
