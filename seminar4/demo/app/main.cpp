#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "broker_csv_source.hpp"
#include "failover_source.hpp"
#include "iss_source.hpp"
#include "quality.hpp"

int main(int argc, char* argv[]) {
    std::filesystem::path data_dir = RP_DATA_DIR;
    std::optional<std::size_t> iss_fail_after;

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--iss-fail-after" && i + 1 < argc) {
            iss_fail_after = std::stoul(argv[++i]);
        } else if (arg == "--data-dir" && i + 1 < argc) {
            data_dir = argv[++i];
        } else {
            std::cerr << "usage: rp_cli [--iss-fail-after N] [--data-dir DIR]\n";
            return 2;
        }
    }

    try {
        rp::FailoverSource source(
            std::make_unique<rp::IssSource>(data_dir / "iss_SBER.csv", iss_fail_after),
            std::make_unique<rp::BrokerCsvSource>(data_dir / "broker_SBER.csv"));

        std::vector<rp::Bar> bars;
        std::map<std::string, int> by_source;
        while (auto bar = source.next()) {
            ++by_source[bar->source];
            bars.push_back(std::move(*bar));
        }

        constexpr std::int64_t minute_ms = 60'000;
        std::cout << "bars=" << bars.size() << " (iss=" << by_source["iss"]
                  << ", broker=" << by_source["broker"] << ")"
                  << " gaps=" << rp::count_gaps(bars, minute_ms)
                  << " source_of_last=" << (bars.empty() ? "none" : bars.back().source) << '\n';
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
