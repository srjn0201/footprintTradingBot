#include "indicators.h"
#include "../../dataStructure.h"
#include <cmath>
#include <vector>
#include <numeric>


// we are using simple linear regression to calculate the slope of cumulative delta over the last 5 bars
// slope m = (N*Σ(xy) - Σx*Σy) / (N*Σ(x^2) - (Σx)^2)
// where x is the bar index (0 to 4) and y is the cumulative delta value of the bar

void calculateCumDelta5barsSlope(Contract& contract) {
    auto& day = contract.weeks.back().days.back();
    if (day.bars.size() < 5) {
        day.cumDelta5barSlope = 0.0;
        return;
    }

    double y1 = day.bars[day.bars.size() - 5].cumDeltaAtBar;
    double y2 = day.bars[day.bars.size() - 4].cumDeltaAtBar;
    double y3 = day.bars[day.bars.size() - 3].cumDeltaAtBar;
    double y4 = day.bars[day.bars.size() - 2].cumDeltaAtBar;
    double y5 = day.bars[day.bars.size() - 1].cumDeltaAtBar;

    double sumY = y1 + y2 + y3 + y4 + y5;
    double sumXY = (0 * y1) + (1 * y2) + (2 * y3) + (3 * y4) + (4 * y5);

    day.cumDelta5barSlope = (5.0 * sumXY - 10.0 * sumY) / 50.0;
    
}