#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace {

constexpr std::size_t default_rows = 100000;
constexpr std::size_t repetitions = 7;
volatile std::uint64_t observable_checksum = 0;

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

struct Trade {
    std::int64_t time_us;
    std::uint64_t symbol_id;
    std::int64_t price_ticks;
    std::int64_t qty;
    char side;
};

bool operator==(const Trade& a, const Trade& b) {
    return a.time_us == b.time_us && a.symbol_id == b.symbol_id &&
           a.price_ticks == b.price_ticks && a.qty == b.qty && a.side == b.side;
}

std::optional<Trade> parse_owning(std::string_view);
std::optional<Trade> parse_views(std::string_view);

void check_contract() {
    const std::array<std::string_view, 5> valid = {
        "0;SBER;1;1;B", "123456;GAZP;25001;10;S", "001;A;002;003;B",
        "9223372036854775807;ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEF;9223372036854775807;9223372036854775807;S",
        "5;X;17;20;B"
    };
    for (const auto row : valid) {
        const auto a = parse_owning(row);
        const auto b = parse_views(row);
        require(a.has_value() && b.has_value(), "valid row rejected: " + std::string(row));
        require(*a == *b, "parser disagreement: " + std::string(row));
    }
    const std::array<std::string_view, 24> invalid = {
        "", "1;SBER;1;1", "1;SBER;1;1;B;extra", "1;SBER;1;1;B;",
        "1;;1;1;B", "1;Sber;1;1;B", "1;SBER1;1;1;B", "1;S BER;1;1;B",
        "-1;SBER;1;1;B", "+1;SBER;1;1;B", " 1;SBER;1;1;B", "1x;SBER;1;1;B",
        "1;SBER;0;1;B", "1;SBER;-1;1;B", "1;SBER;1;0;B", "1;SBER;1;-1;B",
        "1;SBER;1.5;1;B", "1;SBER;1;1x;B", "1;SBER;1;1;X", "1;SBER;1;1;",
        "9223372036854775808;SBER;1;1;B", "1;SBER;9223372036854775808;1;B",
        "1;SBER;1;9223372036854775808;B", "1;SBER;1;1;B\n"
    };
    for (const auto row : invalid) {
        require(!parse_owning(row) && !parse_views(row), "invalid row accepted: " + std::string(row));
    }
    auto source = std::string("7;SBER;101;4;S");
    const auto saved = parse_views(source);
    source.assign(200, 'X');
    require(saved && saved->time_us == 7 && saved->price_ticks == 101 &&
            saved->qty == 4 && saved->side == 'S', "parsed result depends on source lifetime");
    std::cout << "checks: OK (5 valid, 24 invalid, source lifetime)\n";
}

struct FieldAllocations {
    std::size_t calls = 0;
    std::size_t bytes = 0;
};

FieldAllocations field_allocations;

template <class T>
struct CountingAllocator {
    using value_type = T;
    CountingAllocator() = default;
    template <class U>
    CountingAllocator(const CountingAllocator<U>&) noexcept {}
    T* allocate(std::size_t count) {
        T* result = std::allocator<T>{}.allocate(count);
        ++field_allocations.calls;
        field_allocations.bytes += count * sizeof(T);
        return result;
    }
    void deallocate(T* pointer, std::size_t count) noexcept {
        std::allocator<T>{}.deallocate(pointer, count);
    }
};

template <class T, class U>
bool operator==(const CountingAllocator<T>&, const CountingAllocator<U>&) { return true; }

template <class T, class U>
bool operator!=(const CountingAllocator<T>&, const CountingAllocator<U>&) { return false; }

bool parse_integer(std::string_view field, std::int64_t& value) {
    if (field.empty()) {
        return false;
    }
    const auto result = std::from_chars(field.data(), field.data() + field.size(), value);
    return result.ec == std::errc{} && result.ptr == field.data() + field.size();
}

std::uint64_t symbol_hash(std::string_view symbol) {
    std::uint64_t value = 14695981039346656037ULL;
    for (const unsigned char letter : symbol) {
        value ^= letter;
        value *= 1099511628211ULL;
    }
    return value;
}

std::optional<Trade> convert_fields(const std::array<std::string_view, 5>& fields) {
    Trade trade{};
    if (!parse_integer(fields[0], trade.time_us) || trade.time_us < 0 ||
        !parse_integer(fields[2], trade.price_ticks) || trade.price_ticks <= 0 ||
        !parse_integer(fields[3], trade.qty) || trade.qty <= 0 || fields[1].empty() ||
        fields[4].size() != 1 || (fields[4][0] != 'B' && fields[4][0] != 'S')) {
        return std::nullopt;
    }
    for (const char letter : fields[1]) {
        if (letter < 'A' || letter > 'Z') {
            return std::nullopt;
        }
    }
    trade.symbol_id = symbol_hash(fields[1]);
    trade.side = fields[4][0];
    return trade;
}

template <class Allocator>
std::optional<Trade> parse_owning_with(std::string_view line) {
    using Field = std::basic_string<char, std::char_traits<char>, Allocator>;
    std::istringstream input{std::string(line)};
    std::array<Field, 5> fields;
    for (auto& field : fields) {
        if (!std::getline(input, field, ';')) {
            return std::nullopt;
        }
    }
    if (!input.eof()) {
        return std::nullopt;
    }
    std::array<std::string_view, 5> views;
    for (std::size_t i = 0; i < fields.size(); ++i) {
        views[i] = std::string_view(fields[i].data(), fields[i].size());
    }
    return convert_fields(views);
}

std::optional<Trade> parse_owning(std::string_view line) {
    return parse_owning_with<std::allocator<char>>(line);
}

std::optional<Trade> parse_views(std::string_view line) {
    std::array<std::string_view, 5> fields;
    std::size_t begin = 0;
    for (std::size_t i = 0; i < 4; ++i) {
        const auto end = line.find(';', begin);
        if (end == std::string_view::npos) {
            return std::nullopt;
        }
        fields[i] = line.substr(begin, end - begin);
        begin = end + 1;
    }
    fields[4] = line.substr(begin);
    return convert_fields(fields);
}

std::uint64_t trade_checksum(const Trade& trade) {
    return static_cast<std::uint64_t>(trade.time_us) * 31ULL + trade.symbol_id +
           static_cast<std::uint64_t>(trade.price_ticks) * 17ULL +
           static_cast<std::uint64_t>(trade.qty) * 7ULL + static_cast<unsigned char>(trade.side);
}

std::vector<std::string> generate_rows(std::size_t count, bool long_symbols) {
    const std::array<std::string, 4> symbols = long_symbols
        ? std::array<std::string, 4>{"ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEF", "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEG",
                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEH", "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEI"}
        : std::array<std::string, 4>{"SBER", "GAZP", "LKOH", "VTBR"};
    std::vector<std::string> rows;
    rows.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        rows.push_back(std::to_string(1000000 + i * 100) + ';' + symbols[i % symbols.size()] + ';' +
                       std::to_string(100000 + i % 500) + ';' + std::to_string(1 + i % 100) + ';' +
                       (i % 2 == 0 ? 'B' : 'S'));
    }
    return rows;
}

template <class Parser>
std::uint64_t parse_all(const std::vector<std::string>& rows, Parser parse) {
    std::uint64_t checksum = 0;
    for (const auto& row : rows) {
        const auto trade = parse(row);
        if (!trade) {
            throw std::runtime_error("generated row rejected");
        }
        checksum += trade_checksum(*trade);
    }
    observable_checksum = checksum;
    return checksum;
}

template <class Parser>
void benchmark(const char* name, const std::vector<std::string>& rows, Parser parse,
               std::uint64_t expected) {
    require(parse_all(rows, parse) == expected, "warmup checksum mismatch");
    std::array<double, repetitions> samples{};
    for (auto& sample : samples) {
        const auto start = std::chrono::steady_clock::now();
        const auto checksum = parse_all(rows, parse);
        const auto stop = std::chrono::steady_clock::now();
        require(checksum == expected, "timed checksum mismatch");
        sample = std::chrono::duration<double, std::nano>(stop - start).count() / rows.size();
    }
    std::sort(samples.begin(), samples.end());
    const auto median = samples[repetitions / 2];
    std::cout << name << ": median_ns/row=" << median << " rows/s=" << 1e9 / median
              << " checksum=" << expected << '\n';
}

}

int main(int argc, char** argv) {
    try {
        bool check_only = false;
        bool long_symbols = false;
        std::size_t count = default_rows;
        for (int i = 1; i < argc; ++i) {
            const std::string_view argument(argv[i]);
            if (argument == "--check") {
                check_only = true;
            } else if (argument == "--quick") {
                count = 10000;
            } else if (argument == "--long-symbols") {
                long_symbols = true;
            } else if (argument == "--help") {
                std::cout << "parse_bench [--check] [--quick] [--long-symbols]\n";
                return 0;
            } else {
                throw std::runtime_error("unknown argument: " + std::string(argument));
            }
        }
        check_contract();
        if (check_only) {
            return 0;
        }
        const auto rows = generate_rows(count, long_symbols);
        for (const auto& row : rows) {
            const auto owning = parse_owning(row);
            const auto views = parse_views(row);
            require(owning && views && *owning == *views, "generated row parser mismatch");
        }
        const auto expected = parse_all(rows, parse_views);
        std::cout << std::fixed << std::setprecision(2)
                  << "rows=" << count << " symbols=" << (long_symbols ? "32_chars" : "4_chars")
                  << " warmup=1 repeats=" << repetitions << '\n';
        benchmark("owning_getline", rows, parse_owning, expected);
        benchmark("views_from_chars", rows, parse_views, expected);
        field_allocations = {};
        const auto counted = parse_all(rows, parse_owning_with<CountingAllocator<char>>);
        require(counted == expected, "counted parser checksum mismatch");
        const auto owning_allocations = field_allocations;
        field_allocations = {};
        require(parse_all(rows, parse_views) == expected, "view allocation pass mismatch");
        std::cout << "owning_fields_only: field_alloc_calls=" << owning_allocations.calls
                  << " field_alloc_bytes=" << owning_allocations.bytes << '\n'
                  << "view_fields_only: field_alloc_calls=" << field_allocations.calls
                  << " field_alloc_bytes=" << field_allocations.bytes << '\n'
                  << "allocation scope: field strings only; stream buffers and runtime excluded\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
