#include <cstdint>
#include <iostream>

int main() {
    const std::int64_t bid_ticks = 10000;
    const std::int64_t ask_ticks = 10003;

    std::cout << "spread = " << ask_ticks - bid_ticks << " ticks\n";

	return 0;
}
