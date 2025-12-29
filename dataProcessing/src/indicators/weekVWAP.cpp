#include <iostream>
#include <vector>
#include <string>
#include <cmath>     // For std::sqrt
#include <iomanip>   // For std::setprecision
#include "dataStructure.h"


void calculateWeekVWAP(Contract& contract) {
    auto& WEEK = contract.weeks.back();
    auto& lastDAY = WEEK.days.back();
     

    // --- VWAP & Std Dev Calculation ---
    // Only proceed if the day has volume
    if (lastDAY.totalVolume > 0) {

        // 1. Calculate new bar's values
        double typicalPrice = (lastDAY.dayHigh + lastDAY.dayLow + lastDAY.dayClose) / 3.0;
        double priceVolume = typicalPrice * lastDAY.totalVolume;
        
        // 2. Update cumulative state
        WEEK.totalVolume += lastDAY.totalVolume;
        WEEK.cumulativePV += priceVolume;
        
        // 3. Calculate new VWAP
        // (Check day.totalVolume to avoid division by zero)
        if (WEEK.totalVolume > 0) {
            WEEK.vwap = WEEK.cumulativePV / WEEK.totalVolume;
        } else {
            WEEK.vwap = typicalPrice; // On first bar, VWAP is just its price
        }

        // --- Standard Deviation Calculation ---
        // 4. Update cumulative square state
        // This is: Volume * (TypicalPrice * TypicalPrice)
        double squarePriceVolume = lastDAY.totalVolume * typicalPrice * typicalPrice;
        WEEK.cumulativeSquarePV += squarePriceVolume;

        // 5. Calculate Variance
        // Variance = (Sum[V * TP^2] / Sum[V]) - (VWAP^2)
        double variance = (WEEK.cumulativeSquarePV / WEEK.totalVolume) - (WEEK.vwap * WEEK.vwap);

        // 6. Calculate Std Dev and update bands
        // (Check variance > 0; it can be tiny negative from float error)
        if (variance > 0) {
            double stdDev = std::sqrt(variance);
            WEEK.vwapUpperStdDev1 = WEEK.vwap + stdDev;
            WEEK.vwapLowerStdDev1 = WEEK.vwap - stdDev;
        } else {
            // No variance yet (or first bar), so bands are just the VWAP
            WEEK.vwapUpperStdDev1 = WEEK.vwap;
            WEEK.vwapLowerStdDev1 = WEEK.vwap;
        }
        WEEK.vwapBandWidth = WEEK.vwapUpperStdDev1 - WEEK.vwapLowerStdDev1;

    }   // These are complex and require their own state/logic.
}
