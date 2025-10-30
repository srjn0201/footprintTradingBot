#include "../dataStructure.h"
#include "../src/indicators/indicators.h"
#include "../src/TPO/TPO.h"


// this function called at the last tick of the bar to calculate the bar change features
    // feature to be calculated 
    // indicators
        // rsi
        // vWap
        // bollinger bandds
        // daily tpo(volume profile)
    // basic
        // high
        // low
        // close


void updateBarChangeSensitiveFeatures(Contract& contract, double currentPrice, int currentAskVolume, int currentBidVolume) {
    // Update bar change sensitive features
    auto& Day = contract.weeks.back().days.back();
    auto& Bar = Day.bars.back();
    
    // updating the day basics
    Day.dayHigh = std::max(Day.dayHigh, currentPrice);
    Day.dayLow = std::min(Day.dayLow, currentPrice);
    Day.dayClose = currentPrice;

    // calculating the indicators 
    calculateVWAP(contract);
    calculateBBands(contract);
    calculateRSI(contract);
    calculateDayTPO(contract);



    
    
}