#pragma once

#include <charconv>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace rp::csv {

inline std::vector<std::string_view> split(std::string_view line, char sep) {
    std::vector<std::string_view> fields;
    std::size_t start = 0;
	
    while (true) 
	{
        const std::size_t pos = line.find(sep, start);
        fields.push_back(line.substr(start, pos - start));
    
		if (pos == std::string_view::npos) 
		{
            return fields;
        }
        
		start = pos + 1;
    }
}

inline std::int64_t to_int(std::string_view s) {
    std::int64_t value = 0;
    const auto [end, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    
	if (ec != std::errc{} || end != s.data() + s.size()) 
	{
        throw std::invalid_argument("not an integer: '" + std::string(s) + "'");
    }
    
	return value;
}

inline std::int64_t price_to_ticks(std::string_view s, int decimals = 2) {
    const std::size_t dot = s.find('.');
    std::string_view frac = dot == std::string_view::npos ? std::string_view{} : s.substr(dot + 1);
    
	if (frac.size() > static_cast<std::size_t>(decimals)) 
	{
        throw std::invalid_argument("price finer than tick: '" + std::string(s) + "'");
    }
    std::int64_t ticks = to_int(s.substr(0, dot));
    
	for (int i = 0; i < decimals; ++i) 
	{
        const auto idx = static_cast<std::size_t>(i);
        ticks = ticks * 10 + (idx < frac.size() ? to_int(frac.substr(idx, 1)) : 0);
    }
    
	return ticks;
}

inline std::int64_t to_epoch_ms(int y, int mon, int d, int h, int min, int sec) {
    using namespace std::chrono;
    const year_month_day date{year{y}, month{static_cast<unsigned>(mon)},
                              day{static_cast<unsigned>(d)}};
    
	if (!date.ok()) 
	{
        throw std::invalid_argument("bad date");
    }
    const auto tp = sys_days{date} + hours{h} + minutes{min} + seconds{sec};
    
	return duration_cast<milliseconds>(tp.time_since_epoch()).count();
}

inline std::string_view chomp(std::string_view line) {
    if (!line.empty() && line.back() == '\r') 
	{
        line.remove_suffix(1);
    }

    return line;
}

}  // namespace rp::csv
