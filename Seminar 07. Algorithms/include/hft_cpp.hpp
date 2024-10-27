#ifndef HFT_CPP_HPP
#define HFT_CPP_HPP

#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

class NotImplemented : public std::logic_error
{
public:
    NotImplemented() : std::logic_error("Function not yet implemented") { };
};

struct LimitOrder {
    int id;
    double price;
    double volume;
    bool is_buy;
};

struct MarketOrder {
    double volume;
    bool is_buy;
};

/* class BidComparator {
    public:
        bool operator()(int price1, int price2) {
            return (price1 > price2);
        }
        }; */

class OrderBook {
    private:
        int order_id_counter = 0;
        double price_level_step = 0.0025;

        // STL - standard template library
        std::map<int, std::vector<LimitOrder>> ask;
        std::map<int, std::vector<LimitOrder>> bid; //use reverse iterators

    public:
        void add_order(double price, double volume, bool is_buy);
        void remove_order(int id) { throw NotImplemented(); };
        void execute_market_order(MarketOrder order);

        friend std::ostream& operator<<(std::ostream& stream, const OrderBook& se) { throw NotImplemented(); };
};

class Command { // ACTION TICKER VOLUME OPTIONAL<PRICE>
    public:
        std::string action; // BUY SELL REMOVE
        std::string ticker;
        double volume;
        std::optional<double> price;

        friend std::ostream& operator<<(std::ostream& stream, const Command& cmd) { throw NotImplemented(); };
};

std::optional<Command> parse_to_command(const std::string &input);

struct StockExchange {
    private:
        std::unordered_map<std::string, OrderBook> order_books;

    public:
        std::unordered_map<std::string, OrderBook> get_order_books() { return order_books; };
        void process_command(const Command &command);
        friend std::ostream& operator<<(std::ostream& stream, const StockExchange& se) { throw NotImplemented(); };
};

#endif // HFT_CPP_HPP
