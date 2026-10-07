#include "iss_source.hpp"

#include <fstream>
#include <stdexcept>
#include <string_view>

#include "csv_util.hpp"

namespace rp {

namespace {

// open;close;high;low;value;volume;begin;end   begin = "2026-10-02 10:00:00"
Bar parse_iss_line(std::string_view line) {
    const auto f = csv::split(line, ';');
    if (f.size() != 8) 
	{
        throw std::runtime_error("ISS: expected 8 fields, got " + std::to_string(f.size()));
    }
    
	const std::string_view begin = f[6];
    
	Bar bar;
    bar.ts_ms = csv::to_epoch_ms(
        static_cast<int>(csv::to_int(begin.substr(0, 4))), static_cast<int>(csv::to_int(begin.substr(5, 2))),
        static_cast<int>(csv::to_int(begin.substr(8, 2))), static_cast<int>(csv::to_int(begin.substr(11, 2))),
        static_cast<int>(csv::to_int(begin.substr(14, 2))), static_cast<int>(csv::to_int(begin.substr(17, 2))));
    bar.open = csv::price_to_ticks(f[0]);
    bar.close = csv::price_to_ticks(f[1]);
    bar.high = csv::price_to_ticks(f[2]);
    bar.low = csv::price_to_ticks(f[3]);
    bar.volume = csv::to_int(f[5]);
    bar.source = "iss";

    return bar;
}

}  // namespace

IssSource::IssSource(const std::filesystem::path& csv, std::optional<std::size_t> fail_after)
    : fail_after_(fail_after) {
    std::ifstream in(csv);
    if (!in) {
        throw std::runtime_error("cannot open " + csv.string());
    }
    std::string line;
    std::getline(in, line);
    while (std::getline(in, line)) {
        if (!csv::chomp(line).empty()) {
            bars_.push_back(parse_iss_line(csv::chomp(line)));
        }
    }
}

std::optional<Bar> IssSource::next() {
    if (fail_after_ && pos_ >= *fail_after_) {
        throw SourceError("HTTP 503 from MOEX ISS (simulated)");
    }
    if (pos_ == bars_.size()) {
        return std::nullopt;
    }
    return bars_[pos_++];
}

}  // namespace rp
