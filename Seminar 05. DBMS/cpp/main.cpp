
#include <clickhouse/client.h>

#include <iostream>

using namespace clickhouse;

int main() {
    Client client(ClientOptions().SetHost("localhost"));

    client.Select(
        "SELECT price, qty, time FROM trades LIMIT 10", [](const Block& block) {
            for (size_t i = 0; i < block.GetRowCount(); ++i) {
                std::cout << block[0]->As<ColumnDecimal>()->At(i) << ' '
                          << block[1]->As<ColumnDecimal>()->At(i) << ' '
                          << block[2]->As<ColumnDateTime64>()->At(i) << '\n';
            }
        });
    return 0;
}
