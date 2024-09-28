#include <hft_cpp/hft_data_loader.h>

int main() {

    auto order_book = OrderBook(std::string("2024/09/28 19:37:09.364"), OrderBookSide(12.3, 100.), OrderBookSide(12.2, 110.));

    order_book.print();

    return 0;
}
