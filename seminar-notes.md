# Seminar 01
## Outlook
1. Course information:
1.1 Syllabus overview, Grading,
1.2 Course restrictions;
2. Project comments;
3. Prod-like open-source projects;
4. Base repository setup.

# Seminar 2
1.

# Seminar 3
1. Core Concept
* Order book: records active buy (bid) & sell (ask) limit orders.
* Price priority, then time priority if same price.
* Top of book = best bid / best ask.
* Levels:
  * L1 = best bid/ask,
  * L2 = aggregated per price level,
  * L3 = full per-order detail (ids, quantities, updates).

13.0200 = 2000
13.0050 = 100
13.0025 = 12000
13.0000 = 1000

2. Interface Sketch

```cpp
class OrderBookL3 {
 public:
  enum class Action { Add, Cancel, Trade, Replace, Unknown };
  enum class Side { Bid, Ask };

  struct OrderKey { string id; Side side; double price; int64_t qty; int64_t ts; uint64_t seq; };
  struct Trade    { string trade_id; double price; int64_t qty; int64_t ts; uint64_t seq; };
  struct Level    { double price; int64_t agg_qty; size_t count; };

  optional<Trade> OnRecord(const OrderKey&, Action, const string* trade_id = nullptr);
  void LoadCsv(const string& path, const CsvSpec& spec);
  optional<Level> bestBid() const;
  optional<Level> bestAsk() const;
  void ForEachLevel(Side, function<void(const Level&)>) const;
  int64_t openQty(const string& order_id) const;
};
```
	•	OnRecord handles adds, cancels, trades (dedup).
	•	LoadCsv ingests a log file, filtering only one ticker.
	•	bestBid / bestAsk = top levels.
	•	ForEachLevel iterates all price levels.
	•	openQty returns remaining open quantity for given order id.

3. Key std / C++ Tools
	•	`std::map<double, list<Order>>` → sorted price levels (ascending).
	•	`map::rbegin()` + `prev(end())` → highest price in ascending map.
	•	`std::list` to preserve insertion (FIFO) within same price.
	•	`std::unordered_map<orderId, IndexEntry>` → fast order lookup & removal.
	•	`std::unordered_set<tradeId>` → deduplicate matched trades.
	•	`std::from_chars` & `std::string_view` → fast parsing from CSV.
	•	`std::optional<T>` → represent “maybe a value” returns cleanly.
	•	`std::function<void(...)>` → for iteration callbacks across levels.

4. Impl / Pimpl Pattern (Optional)
	•	Public class holds only a pointer to an internal Impl class (forward-declared).
	•	Impl (defined in .cpp) holds all data: maps, lists, indices, logic.
	•	Public methods forward to pimpl_->… implementations.
	•	Benefits: hides implementation, reduces recompilation dependencies, maintains ABI stability.
	•	Downsides: pointer indirection, extra boilerplate.
