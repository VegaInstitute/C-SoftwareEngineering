#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <stdexcept>
#include <utility>

namespace book_exercise {

using Number = std::int64_t;
using Level = std::pair<Number, Number>;
using Book = std::map<Number, Number>;

void bad_accumulate_set(Book& book, Number price, Number qty) {
    book[price] += qty;
}

Number bad_read_quantity(Book& book, Number price) {
    return book[price];
}

Level bad_best_without_empty_check(const Book& book) {
    return *book.rbegin();
}

void set(Book&, Number, Number) {
    throw std::logic_error("TODO: replace absolute quantity; erase zero levels");
}

std::optional<Level> best_bid(const Book&) {
    throw std::logic_error("TODO: return max price, or nullopt for an empty book");
}

Number quantity(const Book&, Number) {
    throw std::logic_error("TODO: read without inserting a missing price");
}

constexpr std::array<Level, 5> events{{
    {100, 5},
    {102, 3},
    {101, 7},
    {102, 0},
    {100, 9}
}};

}
