#include <cmath>
#include <numeric>
#include "indicators.h"
#include "../../dataStructure.h"

double calculatePriceCumDeltaDivergence(Contract& contract, int lookbackBars) {
    auto& day = contract.weeks.back().days.back();
    auto& bars = day.bars;
    int n = bars.size();
    if (n < lookbackBars) return 0.0;

    // 1. Calculate typical Volume Intensity (O(1) update suggested elsewhere, 
    // but here is the logic for the scaler)
    // We use absolute delta to understand the 'force' regardless of direction.
    double currentAbsDelta = std::abs(static_cast<double>(bars.back().delta));
    
    // Simple EMA to smooth the volume scaler
    if (day.avgAbsDelta10 == 0.0) day.avgAbsDelta10 = currentAbsDelta;
    else day.avgAbsDelta10 = (day.avgAbsDelta10 * 9.0 + currentAbsDelta) / 10.0;

    // 2. Linear Regression Slopes (Same as your previous code)
    double sumY_p = 0, sumXY_p = 0, sumY_c = 0, sumXY_c = 0;
    for (int i = 0; i < lookbackBars; ++i) {
        int idx = n - lookbackBars + i;
        sumY_p += bars[idx].close;
        sumXY_p += (i * bars[idx].close);
        sumY_c += bars[idx].cumDeltaAtBar;
        sumXY_c += (i * bars[idx].cumDeltaAtBar);
    }

    double denominator = (lookbackBars == 5) ? 50.0 : 330.0;
    double sumX = (lookbackBars == 5) ? 10.0 : 45.0;

    double pSlope = (lookbackBars * sumXY_p - sumX * sumY_p) / denominator;
    double cSlope = (lookbackBars * sumXY_c - sumX * sumY_c) / denominator;

    // 3. APPLY DYNAMIC SCALES
    // Price Scale: Based on your 2.5 range bar
    double priceScale = 1.0 / 2.5; 

    // CVD Scale: Normalized by the current volume regime
    // If avgAbsDelta10 is 0 (no volume), we avoid division by zero.
    double cvdScale = (day.avgAbsDelta10 > 0) ? (1.0 / day.avgAbsDelta10) : 0.001;

    // 4. Convert to Angles
    double pAngle = std::atan(pSlope * priceScale) * (180.0 / M_PI);
    double cAngle = std::atan(cSlope * cvdScale) * (180.0 / M_PI);

    // 5. Angular Divergence Result
    // Return the delta between the two angles
    // Bullish: cAngle is high (positive), pAngle is low (negative) -> Large Positive Result
    // Bearish: cAngle is low (negative), pAngle is high (positive) -> Large Negative Result
    return cAngle - pAngle;
}