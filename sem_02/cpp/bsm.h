#pragma once

namespace bsm {
    class BSModelParams {
        double vola;

    public:
        BSModelParams(double volatility);
        double total_variance(double dt_years) const;
    };
}
