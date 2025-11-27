import build.pylibbacktester as lb


class MyStrat(lb.Strategy):
    """
    Simple Python strategy example:

    - Ignores book updates and timestamps.
    - On every N-th timer tick, emits a single buy intent of fixed size.
    - Stops after max_slices emissions.
    """

    def __init__(self,
                 strategy_id: str = "py_example",
                 tick_period: int = 10,
                 slice_qty: int = 10,
                 max_slices: int = 5):
        super().__init__()

        self.strategy_id = strategy_id
        self.tick_period = max(1, int(tick_period))
        self.slice_qty = int(slice_qty)
        self.max_slices = int(max_slices)

        self._tick = 0
        self._emitted_slices = 0
        self._pending: list[lb.OrderIntent] = []

    def onBookUpdate(self, ts, seq):
        # ts is a SwigPyObject (std::chrono::milliseconds wrapper) in current setup.
        # For now we ignore it to avoid type issues.
        pass

    def onTimer(self, ts):
        self._tick += 1

        if self._emitted_slices >= self.max_slices:
            return

        intent = lb.OrderIntent()
        intent.strategy_id = self.strategy_id
        intent.side = lb.OrderSide_kBuy
        intent.qty = self.slice_qty

        self._pending.append(intent)
        self._emitted_slices += 1

    def onFill(self, fill):
        # `fill` is a lb.FillEvent. You can inspect it or track PnL.
        # For now, we do nothing to keep the example minimal.
        pass

    def popPendingIntents(self):
        out = list(self._pending)
        print(f"[MyStrat.popPendingIntents : {self.strategy_id}] {len(out)} intents emitted")
        self._pending.clear()
        return out


def run_with_my_strat():
    data_csv = "data/ordlog_sample.csv"
    outdir = "out_py_strategy"
    instrument = "CNYRUB_TOM"

    strat = MyStrat(
        strategy_id="py_example",
        tick_period=10,
        slice_qty=10,
        max_slices=50,
    )

    # Build RunSpec
    spec = lb.RunSpec()
    spec.data_csv_path = data_csv
    spec.instrument = instrument
    spec.out_path = outdir
    spec.tick_ms = 1
    spec.strategy_name = ""

    # If RunSpec has TWAPParams field even for non-TWAP, set a safe default:
    if hasattr(spec, "twap"):
        spec.strategy_params.strategy_id = "twap_dummy"
        spec.strategy_params.side = lb.OrderSide_kSell
        spec.strategy_params.total_qty = 1
        spec.strategy_params.start_ts_ms = 0
        spec.strategy_params.end_ts_ms = 1
        spec.strategy_params.slices = 10
        spec.strategy_params.force_all_equal = True

    res: lb.RunResult = lb.run_backtest(spec, strat)

    print("Backtest finished:")
    print("  outdir:        ", res.out_path)
    print("  trades_csv:    ", res.trades_csv_path)
    print("  equity_csv:    ", res.equity_csv_path)
    print("  top_of_book:   ", res.top_of_the_book_csv_path)


if __name__ == "__main__":
    run_with_my_strat()
