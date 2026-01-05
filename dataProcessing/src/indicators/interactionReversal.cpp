#include <cmath>
#include <numeric>
#include "indicators.h"
#include "../../dataStructure.h"


void calculateInteractionReversal(Contract& contract) {
    auto& day = contract.weeks.back().days.back();
    if (day.bars.empty()) return;

    auto& currentBar = day.bars.back();

    // 1. Calculate the raw distance from VWAP
    // We use the 'close' price of the bar
    double vwapDist = currentBar.close - day.vwap;

    // 2. Interaction Reversal Calculation
    // We use your 11-bar Z-score
    day.interactionReversal = day.deltaZscore11bars * vwapDist;

    // 3. Optional: Update a rolling average (O(1) EMA style)
    // This helps the bot understand if the current reversal signal 
    // is 'big' compared to the last 20 bars.
    if (day.interactionReversal20barAvg == 0.0) {
        day.interactionReversal20barAvg = std::abs(day.interactionReversal);
    } else {
        day.interactionReversal20barAvg = (day.interactionReversal20barAvg * 19.0 + 
                                          std::abs(day.interactionReversal)) / 20.0;
    }
}