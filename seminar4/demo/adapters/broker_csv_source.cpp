#include "broker_csv_source.hpp"

#include <fstream>
#include <stdexcept>
#include <string_view>

#include "csv_util.hpp"

namespace rp {

namespace {

// SBER,1,20261002,100000,285.43,285.60,285.40,285.50,43210
Bar parse_broker_line(std::string_view line) {
    const auto f = csv::split(line, ',');
    
	if (f.size() != 9) 
	{
        throw std::runtime_error("broker CSV: expected 9 fields, got " + std::to_string(f.size()));
    }
    
	const auto date = csv::to_int(f[2]);  // YYYYMMDD
    const auto time = csv::to_int(f[3]);  // HHMMSS
    
	Bar bar;
    bar.ts_ms = csv::to_epoch_ms(static_cast<int>(date / 10000), static_cast<int>(date / 100 % 100),
                                 static_cast<int>(date % 100), static_cast<int>(time / 10000),
                                 static_cast<int>(time / 100 % 100), static_cast<int>(time % 100));
    bar.open = csv::price_to_ticks(f[4]);
    bar.high = csv::price_to_ticks(f[5]);
    bar.low = csv::price_to_ticks(f[6]);
    bar.close = csv::price_to_ticks(f[7]);
    bar.volume = csv::to_int(f[8]);
    bar.source = "broker";

    return bar;
}

}  // namespace

BrokerCsvSource::BrokerCsvSource(const std::filesystem::path& csv) {
    std::ifstream in(csv);
    if (!in) 
	{
        throw std::runtime_error("cannot open " + csv.string());
    }

    std::string line;
    std::getline(in, line);

    while (std::getline(in, line)) 
	{
        if (!csv::chomp(line).empty()) 
		{
            bars_.push_back(parse_broker_line(csv::chomp(line)));
        }
    }
}

std::optional<Bar> BrokerCsvSource::next() {
    if (pos_ == bars_.size()) 
	{
        return std::nullopt;
    }

    return bars_[pos_++];
}

}  // namespace rp
