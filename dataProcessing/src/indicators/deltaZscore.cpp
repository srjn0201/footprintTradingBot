#include "indicators.h"
#include "../../dataStructure.h"
#include <cmath>
#include <vector>
#include <numeric>

void calculateDeltaZscore(Contract& contract) {
    auto& day = contract.weeks.back().days.back();
    int n = day.bars.size();

    if (n == 0) {
        return;
    }

    double newDelta = static_cast<double>(day.bars.back().delta);

    if (n == 1) {
        day.deltaZscore20bars = newDelta;
        day.deltaSum = newDelta;
        day.deltaSumOfSquares = newDelta * newDelta;
        return;
    }

    if (n <= 20) {
        // O(N) calculation for the first 20 bars
        std::vector<double> deltas;
        for (const auto& bar : day.bars) {
            deltas.push_back(static_cast<double>(bar.delta));
        }

        day.deltaSum = std::accumulate(deltas.begin(), deltas.end(), 0.0);
        day.deltaSumOfSquares = 0.0;
        for(double d : deltas) {
            day.deltaSumOfSquares += d * d;
        }

        double mean = day.deltaSum / n;
        double variance = (day.deltaSumOfSquares / n) - (mean * mean);
        double stdDev = (variance > 0) ? std::sqrt(variance) : 0;

        if (stdDev != 0) {
            day.deltaZscore20bars = (newDelta - mean) / stdDev;
        } else {
            day.deltaZscore20bars = 0;
        }
    } else { // n > 20
        // O(1) rolling calculation
        double oldDelta = static_cast<double>(day.bars[n - 21].delta);
        day.deltaSum += newDelta - oldDelta;
        day.deltaSumOfSquares += newDelta * newDelta - oldDelta * oldDelta;

        double mean = day.deltaSum / 20;
        double variance = (day.deltaSumOfSquares / 20) - (mean * mean);
        double stdDev = (variance > 0) ? std::sqrt(variance) : 0;

        if (stdDev != 0) {
            day.deltaZscore20bars = (newDelta - mean) / stdDev;
        } else {
            day.deltaZscore20bars = 0;
        }
    }
}