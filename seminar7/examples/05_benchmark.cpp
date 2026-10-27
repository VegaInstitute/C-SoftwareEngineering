#include <chrono>
#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace benchmark_demo {

using Clock = std::chrono::steady_clock;
using Rows = std::vector<std::string>;
inline volatile std::uint64_t observed_checksum = 0;

template <class ParseRecord>
std::int64_t bad_time_file_and_print(std::istream& file, std::ostream& output,
                                     ParseRecord parse_record) {
    const auto start = Clock::now();
    std::string line;
    while (std::getline(file, line)) {
        output << parse_record(line) << '\n';
    }
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        Clock::now() - start).count();
}

template <class ParsePrice>
std::int64_t bad_time_price_only_unused(const Rows& rows, ParsePrice parse_price) {
    const auto start = Clock::now();
    for (const auto& line : rows) {
        parse_price(line);
    }
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        Clock::now() - start).count();
}

template <class ParseRecord>
std::uint64_t good_checksum(const Rows& rows, ParseRecord parse_record) {
    std::uint64_t checksum = 0;
    for (const auto& line : rows) {
        checksum += parse_record(line);
    }
    return checksum;
}

struct Measurement {
    std::vector<std::int64_t> nanoseconds;
    std::uint64_t checksum;
};

template <class ParseRecord>
Measurement good_measure_pure(const Rows& rows, ParseRecord parse_record,
                              std::size_t repetitions = 7) {
    if (repetitions == 0) {
        throw std::invalid_argument("repetitions must be positive");
    }
    const auto expected = good_checksum(rows, parse_record);
    observed_checksum = expected;
    Measurement result{{}, expected};
    result.nanoseconds.reserve(repetitions);
    for (std::size_t i = 0; i < repetitions; ++i) {
        const auto start = Clock::now();
        const auto checksum = good_checksum(rows, parse_record);
        const auto finish = Clock::now();
        observed_checksum = checksum;
        if (checksum != expected) {
            throw std::runtime_error("parser result changed between repetitions");
        }
        result.nanoseconds.push_back(
            std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count());
    }
    return result;
}

}
