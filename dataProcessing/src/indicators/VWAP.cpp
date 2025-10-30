#include <iostream>
#include <vector>
#include <string>
#include <cmath>     // For std::sqrt
#include <iomanip>   // For std::setprecision
#include "dataStructure.h"


void calculateVWAP(Contract& contract) {
    auto& DAY = contract.weeks.back().days.back();
    auto& lastBAR = DAY.bars.back();
     

    // --- VWAP & Std Dev Calculation ---
    // Only proceed if the bar has volume
    if (lastBAR.barTotalVolume > 0) {

        // 1. Calculate new bar's values
        double typicalPrice = (lastBAR.high + lastBAR.low + lastBAR.close) / 3.0;
        double priceVolume = typicalPrice * lastBAR.barTotalVolume;
        
        // 2. Update cumulative state
        DAY.totalVolume += lastBAR.barTotalVolume;
        DAY.cumulativePV += priceVolume;
        
        // 3. Calculate new VWAP
        // (Check day.totalVolume to avoid division by zero)
        if (DAY.totalVolume > 0) {
            DAY.vwap = DAY.cumulativePV / DAY.totalVolume;
        } else {
            DAY.vwap = typicalPrice; // On first bar, VWAP is just its price
        }

        // --- Standard Deviation Calculation ---
        // 4. Update cumulative square state
        // This is: Volume * (TypicalPrice * TypicalPrice)
        double squarePriceVolume = lastBAR.barTotalVolume * typicalPrice * typicalPrice;
        DAY.cumulativeSquarePV += squarePriceVolume;

        // 5. Calculate Variance
        // Variance = (Sum[V * TP^2] / Sum[V]) - (VWAP^2)
        double variance = (DAY.cumulativeSquarePV / DAY.totalVolume) - (DAY.vwap * DAY.vwap);

        // 6. Calculate Std Dev and update bands
        // (Check variance > 0; it can be tiny negative from float error)
        if (variance > 0) {
            double stdDev = std::sqrt(variance);
            DAY.vwapUpperStdDev1 = DAY.vwap + stdDev;
            DAY.vwapUpperStdDev2 = DAY.vwap + 2 * stdDev;
            DAY.vwapLowerStdDev1 = DAY.vwap - stdDev;
            DAY.vwapLowerStdDev2 = DAY.vwap - 2 * stdDev;
        } else {
            // No variance yet (or first bar), so bands are just the VWAP
            DAY.vwapUpperStdDev1 = DAY.vwap;
            DAY.vwapLowerStdDev1 = DAY.vwap;
            DAY.vwapUpperStdDev2 = DAY.vwap;
            DAY.vwapLowerStdDev2 = DAY.vwap;
        }
    }   // These are complex and require their own state/logic.
}
