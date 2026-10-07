#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "bar_source.hpp"

namespace rp {

class FailoverSource final : public IBarSource {
public:
    FailoverSource(std::unique_ptr<IBarSource> primary, std::unique_ptr<IBarSource> backup)
        : primary_(std::move(primary)), backup_(std::move(backup)) {}

    std::optional<Bar> next() override {
        if (!switched_) {
            try {
                auto bar = primary_->next();
                if (bar) {
                    last_ts_ = bar->ts_ms;
                }
                return bar;
            } catch (const SourceError& e) {
                switched_ = true;
                std::cerr << "WARN: " << primary_->name() << " failed: " << e.what()
                          << " -> switching to " << backup_->name() << '\n';
            }
        }
        while (auto bar = backup_->next()) {
            if (!last_ts_ || bar->ts_ms > *last_ts_) {
                last_ts_ = bar->ts_ms;
                return bar;
            }
        }
        return std::nullopt;
    }

    std::string name() const override {
        return "failover(" + primary_->name() + " -> " + backup_->name() + ")";
    }

    bool switched() const { return switched_; }

private:
    std::unique_ptr<IBarSource> primary_;
    std::unique_ptr<IBarSource> backup_;
    bool switched_ = false;
    std::optional<std::int64_t> last_ts_;
};

}  // namespace rp
