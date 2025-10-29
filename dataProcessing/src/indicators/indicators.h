#ifndef INDICATORS_H
#define INDICATORS_H

#include "../dataStructure.h"
#include <string>
#include <utility>
#include <limits>    // For std::numeric_limits::quiet_NaN




// rsi calculations
    // this will create a single columnin the day struct in the days vector of week struct
    // this will add only one result in the day.RSI
void calculateRSI(Contract& contract);

// bollinger band
void calculateBBands(Contract& contract);



// vwap calculations
void calculateVWAP(Contract& contract);









#endif // INDICATORS_H