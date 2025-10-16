#include "libbacktester/strategies/twap_strategy.hpp"

#include <cassert>
#include <stdexcept>
#include <utility>

namespace libbacktester {

namespace {

// start + floor(i * span / N); last slice maps to end.
// Uses 128-bit intermediate when available to avoid i*span overflow.
inline std::int64_t mul_div_floor(std::size_t i,
                                  std::int64_t span,
                                  std::size_t N,
                                  std::int64_t start,
                                  std::int64_t end,
                                  bool last_slice) noexcept {
  if (last_slice) return end;

#if defined(__SIZEOF_INT128__)
  __int128 prod = static_cast<__int128>(span) * static_cast<__int128>(i);
  __int128 quo  = prod / static_cast<__int128>(N);
  return start + static_cast<std::int64_t>(quo);
#else
  const std::int64_t q = span / static_cast<std::int64_t>(N);
  const std::int64_t r = span % static_cast<std::int64_t>(N);
#ifndef NDEBUG
  const auto i64 = static_cast<std::int64_t>(i);
  if (r > 0 && i64 > 0) {
    const auto max = (std::numeric_limits<std::int64_t>::max)();
    assert(r <= max / i64 && "TWAP: (span%N) * i overflow in fallback");
  }
#endif
  const std::int64_t off = q * static_cast<std::int64_t>(i)
                         + (r * static_cast<std::int64_t>(i)) / static_cast<std::int64_t>(N);
  return start + off;
#endif
}

inline bool nondecreasing(std::int64_t a, std::int64_t b) noexcept { return a <= b; }

}  // namespace

TWAPStrategy::TWAPStrategy(std::string strategy_id,
                           OrderSide side,
                           std::int64_t total_qty,
                           Ms start_ts,
                           Ms end_ts,
                           std::size_t slices,
                           bool force_all_equal)
    : strategy_id_(std::move(strategy_id)),
      side_(side),
      total_qty_(total_qty),
      start_ts_(start_ts),
      end_ts_(end_ts),
      slices_(slices),
      force_all_equal_(force_all_equal) {
  if (strategy_id_.empty()) throw std::invalid_argument("TWAP: strategy_id must be non-empty");
  if (total_qty_ <= 0)      throw std::invalid_argument("TWAP: total_qty must be > 0");
  if (slices_ == 0)         throw std::invalid_argument("TWAP: slices must be > 0");
  if (end_ts_ <= start_ts_) throw std::invalid_argument("TWAP: end_ts must be > start_ts");
  buildScheduleAndIntents();
}

void TWAPStrategy::buildScheduleAndIntents() {
  intents_.clear();
  intents_.reserve(slices_);

  const std::int64_t start = start_ts_.count();
  const std::int64_t end   = end_ts_.count();
  const std::int64_t span  = end - start;
  const std::size_t  N     = slices_;

  const std::int64_t base      = total_qty_ / static_cast<std::int64_t>(N);
  const std::int64_t rem_total = total_qty_ % static_cast<std::int64_t>(N);
  std::int64_t       rem_left  = rem_total;

#ifndef NDEBUG
  std::int64_t last_due = start;
#endif

  for (std::size_t i = 1; i <= N; ++i) {
    const bool last = (i == N);
    const std::int64_t due_ms = mul_div_floor(i, span, N, start, end, last);

#ifndef NDEBUG
    assert(nondecreasing(last_due, due_ms) && "TWAP: due times must be non-decreasing");
    last_due = due_ms;
#endif

    std::int64_t qty = base;
    if (force_all_equal_) {
      if (last) qty = total_qty_ - base * static_cast<std::int64_t>(N - 1);
    } else if (rem_left > 0) {
      ++qty;
      --rem_left;
    }

    if (qty <= 0) continue;

    OrderIntent oi;
    oi.strategy_id = strategy_id_;
    oi.local_id.reserve(strategy_id_.size() + 1 + 20);
    oi.local_id = strategy_id_;
    oi.local_id.push_back('-');
    oi.local_id += std::to_string(i);
    oi.side = side_;
    oi.price = std::nullopt;
    oi.qty = qty;
    oi.submit_ts = Ms{due_ms};

    intents_.emplace_back(std::move(oi));
  }

  next_due_index_ = 0;
  last_popped_index_ = 0;
}

void TWAPStrategy::onBookUpdate(Ms /*ts*/, std::uint64_t /*seq*/) noexcept {
  // no-op in S5
}

void TWAPStrategy::onTimer(Ms ts) noexcept {
  const std::size_t n = intents_.size();
  while (next_due_index_ < n &&
         ts >= intents_[next_due_index_].submit_ts) {
    ++next_due_index_;
  }
}

void TWAPStrategy::onFill(const FillEvent& /*fill*/) {
  // not used in S5
}

std::vector<OrderIntent> TWAPStrategy::popPendingIntents() {
  if (last_popped_index_ >= next_due_index_) return {};
  const std::size_t count = next_due_index_ - last_popped_index_;
  std::vector<OrderIntent> out;
  out.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    out.emplace_back(std::move(intents_[last_popped_index_ + i]));
  }
  last_popped_index_ = next_due_index_;
  return out;
}

}  // namespace libbacktester
