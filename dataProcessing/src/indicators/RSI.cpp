#include <iostream>
#include <vector>
#include <cmath>     // For std::isnan
#include <limits>    // For std::numeric_limits::quiet_NaN
#include <tuple>     // For std::tuple, std::make_tuple, std::tie
#include <iomanip>   // For std::setprecision
#include "dataStructure.h"



//  * Calculates or updates the RSI state.
//  * This function has two modes based on the optional parameters:
//  *
//  * 1. INIT MODE: If prevAvgGain or prevAvgLoss is NaN (default),
//  * it calculates the latest RSI from the entire 'PRICES' vector.
//  * - It respects Req 1: If data is low, it uses an effective14.
//  *
//  * 2. UPDATE MODE: If prevAvgGain and prevAvgLoss are valid numbers,
//  * it performs an O(1) update using only the *last two PRICES*
//  * in the 'PRICES' vector.
//  *
//  * @param PRICES A vector of PRICES. In UPDATE_MODE, this must
//  * contain at least 2 PRICES.
//  * @param 14 The desired RSI lookback 14 (e.g., 14).
//  * @param prevAvgGain The Average Gain from the previous bar.
//  * @param prevAvgLoss The Average Loss from the previous bar.
//  * @return RsiResult containing {new_rsi, new_avgGain, new_avgLoss}.
 
void calculateRSI(Contract& contract) {

    auto& day = contract.weeks.back().days.back();
    auto& PRICES = day.bars;
    const double nan = std::numeric_limits<double>::quiet_NaN();


    // --- CASE 1: INITIALIZATION MODE ---
    // If no previous state is provided, calculate from the vector.
    if (std::isnan(day.prevAvgGain) || std::isnan(day.prevAvgLoss) || PRICES.size() < 15){
        
        // --- 1.1. Validation ---
        if (PRICES.size() < 2) {
            day.rsi = nan;
            day.prevAvgGain = nan;
            day.prevAvgLoss = nan;
            return;
        }

        // --- 1.2. Determine Effective 14 (Req 1) ---
        int effective14 = 14;
        size_t dataSize = PRICES.size();
        if (dataSize < static_cast<size_t>(14) + 1) {
            effective14 = dataSize - 1; // Use all available data
        }

        double avgGain = 0.0;
        double avgLoss = 0.0;

        // --- 1.3. Seeding Phase (Calculate first SMA) ---
        for (int i = 1; i <= effective14; ++i) {
            double change = static_cast<double>(PRICES[i].close) - static_cast<double>(PRICES[i - 1].close);
            if (change > 0) {
                avgGain += change;
            } else {
                avgLoss -= change;
            }
        }
        avgGain /= effective14;
        avgLoss /= effective14;

        // --- 1.4. Smoothing Phase (If we have more data) ---
        for (size_t i = static_cast<size_t>(effective14) + 1; i < dataSize; ++i) {
            double change = static_cast<double>(PRICES[i].close) - static_cast<double>(PRICES[i - 1].close);
            double currentGain = (change > 0) ? change : 0.0;
            double currentLoss = (change < 0) ? -change : 0.0;

            // Use the *original desired 14* for smoothing
            avgGain = ((avgGain * (14 - 1)) + currentGain) / 14;
            avgLoss = ((avgLoss * (14 - 1)) + currentLoss) / 14;
        }

        // --- 1.5. Final RSI Calculation ---
        day.prevAvgGain = avgGain;
        day.prevAvgLoss = avgLoss;
        if (avgLoss == 0.0) {
            day.rsi = 100.0;
            return;
        }
        double rs = avgGain / avgLoss;
        day.rsi = 100.0 - (100.0 / (1.0 + rs));
        return;

    } 
    
    // --- CASE 2: UPDATE MODE ---
    // If a valid previous state is provided, perform O(1) update.
    else {
        
        // --- 2.1. Validation ---
        if (PRICES.size() < 2) {
            // Not enough data to get the latest change
            day.rsi = nan;
            day.prevAvgGain = nan;
            day.prevAvgLoss = nan;
            return;
        }

        // --- 2.2. Get Newest Price Change ---
        double newPrice = static_cast<double>(PRICES.back().close);
        double prevPrice = static_cast<double>(PRICES[PRICES.size() - 2].close);
        double change = newPrice - prevPrice;

        double currentGain = (change > 0) ? change : 0.0;
        double currentLoss = (change < 0) ? -change : 0.0;

        // --- 2.3. Apply Wilder's Smoothing (The O(1) Update) ---
        double newAvgGain = ((day.prevAvgGain * (14 - 1)) + currentGain) / 14;
        double newAvgLoss = ((day.prevAvgLoss * (14 - 1)) + currentLoss) / 14;

        // --- 2.4. Final RSI Calculation ---
        day.prevAvgGain = newAvgGain;
        day.prevAvgLoss = newAvgLoss;
        if (newAvgLoss == 0.0) {
            day.rsi = 100.0;
            return;
        }
        double rs = newAvgGain / newAvgLoss;
        day.rsi = 100.0 - (100.0 / (1.0 + rs));
        return;
    }
}