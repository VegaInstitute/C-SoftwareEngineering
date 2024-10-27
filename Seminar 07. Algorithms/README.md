# Seminar 7. Algorithms in Quantitative Analytics – L3 Order Book
## 1. Introduction to the L3 Order Book

###	What is an L3 Order Book?
The L3 (Level 3) order book records individual orders at each price level, showing the full depth and detail of every buy and sell order, including time-stamps, order quantities, and identities of market participants when available.
### Why It’s Important
L3 data provides a granular view, essential for modeling order flow, detecting market-making patterns, and building high-frequency trading (HFT) algorithms.
### Data Complexity
With potentially thousands of updates per second, L3 data requires efficient storage and processing to be useful in real-time.

## 2. Algorithmic Approach to Order Book Construction

### Order Book Data Structure
* Use a sorted data structure (e.g., std::map in C++ or a balanced tree structure) for efficient insertions and deletions.
* A dual-map structure (one for bids, one for asks) can help manage and retrieve orders based on price levels and their respective orders.

### Core Functions
* Add Order: Insert an order into the bid or ask tree based on its price and quantity.
* Cancel Order: Locate and remove an order by its unique identifier.
* Modify Order: Adjust an existing order’s size or price, if required.
* Match Orders: Implement logic to match incoming market orders against resting limit orders.

## 3. Metrics Calculation and Real-time Analytics
### Key Metrics
* Best Bid/Ask: Continuously update the best (highest) bid and best (lowest) ask prices, which reflect the order book’s top levels.
* Spread: The difference between the best ask and best bid, crucial for understanding market liquidity.
* Order Imbalance: The ratio of buy to sell orders within a price range or across the entire book, used for sentiment analysis.
* Price Impact: Assess the price change due to hypothetical trades of varying sizes. This involves calculating the volume-weighted average price (VWAP) over different trade sizes.
* Order Flow and Volume Profiles: Real-time analysis of order flow direction, i.e., whether buy or sell orders dominate, and the distribution of volume across price levels.
### Computational Considerations
* For real-time metrics, maintain cumulative data within the order book structure to avoid recalculating from scratch.
* Use incremental updates for metrics like VWAP, bid/ask spread, and order imbalance for efficiency.


## 4. Practical example: L3 Order Book
In this session I shall cover 3 types of algorithms I find useful:
1. Regular expressions;
2. Functors;
3. Iterators.
