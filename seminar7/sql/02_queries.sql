SELECT trade_id, time_us, symbol_id, price_ticks, qty, side
FROM trades
ORDER BY trade_id;

SELECT trade_id, time_us, price_ticks, qty
FROM trades
WHERE side = 'B'
ORDER BY time_us DESC
LIMIT 2;

SELECT t.trade_id, i.symbol, t.price_ticks, t.qty, t.side
FROM trades AS t
JOIN instruments AS i ON i.symbol_id = t.symbol_id
ORDER BY t.trade_id;

SELECT
    i.symbol,
    SUM(t.qty) AS volume,
    SUM(t.price_ticks::numeric * t.qty) AS amount_ticks,
    ROUND(
        SUM(t.price_ticks::numeric * t.qty) / NULLIF(SUM(t.qty), 0),
        2
    ) AS vwap_ticks
FROM trades AS t
JOIN instruments AS i ON i.symbol_id = t.symbol_id
GROUP BY i.symbol
ORDER BY i.symbol;

BEGIN;

UPDATE trades
SET qty = 3
WHERE trade_id = 1001;

SELECT trade_id, price_ticks, qty
FROM trades
WHERE trade_id = 1001;

ROLLBACK;

SELECT trade_id, price_ticks, qty
FROM trades
WHERE trade_id = 1001;

BEGIN;

INSERT INTO trades (trade_id, time_us, symbol_id, price_ticks, qty, side)
VALUES (1004, 3, 1, 105, 2, 'B');

SELECT trade_id, price_ticks, qty
FROM trades
WHERE trade_id = 1004;

DELETE FROM trades
WHERE trade_id = 1004;

SELECT trade_id, price_ticks, qty
FROM trades
WHERE trade_id = 1004;

ROLLBACK;
