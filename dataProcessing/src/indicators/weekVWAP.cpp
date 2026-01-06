#include <iostream>
#include <vector>
#include <string>
#include <cmath>     // For std::sqrt
#include <iomanip>   // For std::setprecision
#include "dataStructure.h"


void calculateWeekVWAP(Contract& contract) {
    auto& WEEK = contract.weeks.back();
    auto& day = WEEK.days.back(); // Reference the current day
    
    // Get the current bar (the one just processed)
    if (day.bars.empty()) { // Should not happen if called after a bar is added, but good for safety
        return;
    }
    auto& currentBar = day.bars.back();

    // --- VWAP & Std Dev Calculation ---
    // Only proceed if the current bar has volume
    if (currentBar.barTotalVolume > 0) { // Use currentBar.volume, not day.totalVolume

        // 1. Calculate current bar's values
        // VWAP typically uses (high + low + close) / 3 or (open + high + low + close) / 4
        // For simplicity and consistency, let's use the bar's close price as the typical price
        // or (currentBar.high + currentBar.low + currentBar.close) / 3.0; if preferred
        double typicalPrice = currentBar.close; 
        double priceVolume = typicalPrice * currentBar.barTotalVolume; // Use currentBar.volume
        
        // 2. Update cumulative state
        WEEK.totalVolume += currentBar.barTotalVolume; // Add current bar's volume
        WEEK.cumulativePV += priceVolume;      // Add current bar's PV
        
        // 3. Calculate new VWAP
        if (WEEK.totalVolume > 0) {
            WEEK.vwap = WEEK.cumulativePV / WEEK.totalVolume;
        } else {
            // This case should ideally not be reached if currentBar.volume > 0
            // but as a fallback, set to current bar's typical price.
            WEEK.vwap = typicalPrice; 
        }

        // --- Standard Deviation Calculation ---
        // 4. Update cumulative square state for current bar
        // This is: Volume * (TypicalPrice * TypicalPrice) for the current bar
        double squarePriceVolume = currentBar.barTotalVolume * typicalPrice * typicalPrice; // Use currentBar.volume
        WEEK.cumulativeSquarePV += squarePriceVolume;

        // 5. Calculate Variance
        // Variance = (Sum[V * TP^2] / Sum[V]) - (VWAP^2)
        // Ensure WEEK.totalVolume is not zero when calculating variance
        if (WEEK.totalVolume > 0) {
            double variance = (WEEK.cumulativeSquarePV / WEEK.totalVolume) - (WEEK.vwap * WEEK.vwap);

            // 6. Calculate Std Dev and update bands
            // (Check variance > 0; it can be tiny negative from float error)
            const double EPSILON = 1e-9;
            if (variance > EPSILON) { // Use a small epsilon for float comparison
                double stdDev = std::sqrt(variance);
                WEEK.vwapUpperStdDev1 = WEEK.vwap + stdDev;
                WEEK.vwapLowerStdDev1 = WEEK.vwap - stdDev;
            } else {
                // No significant variance, so bands are just the VWAP
                WEEK.vwapUpperStdDev1 = WEEK.vwap;
                WEEK.vwapLowerStdDev1 = WEEK.vwap;
            }
        } else {
            // If total WEEK.totalVolume is 0, reset bands to VWAP
            WEEK.vwapUpperStdDev1 = WEEK.vwap;
            WEEK.vwapLowerStdDev1 = WEEK.vwap;
        }
        WEEK.vwapBandWidth = WEEK.vwapUpperStdDev1 - WEEK.vwapLowerStdDev1;
    }
}
