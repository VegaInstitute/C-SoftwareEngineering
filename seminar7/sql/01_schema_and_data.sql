BEGIN;

CREATE TABLE instruments (
    symbol_id INTEGER PRIMARY KEY,
    symbol TEXT NOT NULL,
    CONSTRAINT instruments_symbol_key UNIQUE (symbol),
    CONSTRAINT instruments_symbol_nonempty CHECK (length(trim(symbol)) > 0)
);

CREATE TABLE trades (
    trade_id BIGINT PRIMARY KEY,
    time_us BIGINT NOT NULL,
    symbol_id INTEGER NOT NULL,
    price_ticks BIGINT NOT NULL,
    qty BIGINT NOT NULL,
    side TEXT NOT NULL,
    CONSTRAINT trades_time_us_nonnegative CHECK (time_us >= 0),
    CONSTRAINT trades_symbol_id_fkey FOREIGN KEY (symbol_id)
        REFERENCES instruments (symbol_id) ON DELETE RESTRICT,
    CONSTRAINT trades_price_ticks_positive CHECK (price_ticks > 0),
    CONSTRAINT trades_qty_positive CHECK (qty > 0),
    CONSTRAINT trades_side_valid CHECK (side IN ('B', 'S'))
);

INSERT INTO instruments (symbol_id, symbol)
VALUES
    (1, 'SBER'),
    (2, 'GAZP');

INSERT INTO trades (trade_id, time_us, symbol_id, price_ticks, qty, side)
VALUES
    (1001, 0, 1, 100, 1, 'B'),
    (1002, 1, 1, 110, 9, 'S'),
    (1003, 2, 2, 200, 2, 'B');

COMMIT;
