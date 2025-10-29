#include <iostream>
#include <vector>
#include <cmath>     // For std::pow and std::sqrt
#include <iomanip>   // For std::setprecision

#include "../dataStructure.h"

void calculateBBands(Contract& contract) {
    // --- Constants ---
    // We use a 21-period lookback, per your rules.
    const int WILDER_PERIOD = 21;
    const double K_MULTIPLIER = 2.4;

    auto& day = contract.weeks.back().days.back();
    int n = day.bars.size();
    

    // Do nothing if the vector is empty
    if (n < 2) {
        return;
    }

    // --- Rule 2: Seeding Phase (2 <= n <= 21) ---
    // We use a simple moving average (SMA) and population variance
    // as the seed for the Wilder's smoothing.
    // This is an O(n) calculation, which is necessary for seeding.
    if (n <= WILDER_PERIOD && n >= 2) {

        // Calculate SMA
        double sum_prices = 0.0;
        for (int i = 0; i < n; ++i) {
            sum_prices += day.bars[i].close;
        }
        day.bbMiddle = sum_prices / n;

        // Calculate Population Variance
        double sum_sq_diff = 0.0;
        for (int i = 0; i < n; ++i) {
            sum_sq_diff += std::pow(day.bars[i].close - day.bbMiddle, 2);
        }
        day.variance = sum_sq_diff / n;

        // 3. Calculate new Upper and Lower Bands
        double std_dev = std::sqrt(day.variance);
        day.bbUpper = day.bbMiddle + (K_MULTIPLIER * std_dev);
        day.bbLower = day.bbMiddle - (K_MULTIPLIER * std_dev);

    }
    
    // --- Rule 3: O(1) Recursive Phase (n > 21) ---
    else {
        // Get the previous bar's state
        // const PriceBar& prev_bar = prices[n - 2];
        double alpha = 1.0 / WILDER_PERIOD;

        // 1. Calculate new Middle Band (Wilder's Smoothing)
        //    Formula: WS_t = WS_{t-1} + alpha * (P_new - WS_{t-1})
        day.bbMiddle = day.bbMiddle + alpha * (day.bars.back().close - day.bbMiddle);

        // 2. Calculate new Exponential Variance
        //    Formula: Var_t = (1-alpha) * Var_{t-1} + alpha * (P_new - WS_t)^2
        //    We use the *new* middle band for the squared difference.
        double sq_diff = std::pow(day.bars.back().close - day.bbMiddle, 2);
        day.variance = (1.0 - alpha) * day.variance + alpha * sq_diff;

        // 3. Calculate new Upper and Lower Bands
        double std_dev = std::sqrt(day.variance);
        day.bbUpper = day.bbMiddle + (K_MULTIPLIER * std_dev);
        day.bbLower = day.bbMiddle - (K_MULTIPLIER * std_dev);
    }

}
