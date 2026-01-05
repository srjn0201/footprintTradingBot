#include "../dataStructure.h"
#include "../src/indicators/indicators.h"
#include "../src/TPO/TPO.h"


// this function called at the last tick of the bar to calculate the bar change features
    // feature to be calculated 
    // indicators
        // rsi(day)
        // vWap(day an dweek)
        // bollinger bandds(day)
        // daily tpo(volume profile)
    // basic(daily and weekly)
        // high
        // low
        // close


void updateBarChangeSensitiveFeatures(Contract& contract, double currentPrice, int currentAskVolume, int currentBidVolume) {
    // Update bar change sensitive features
    auto& WEEK = contract.weeks.back();
    auto& Day = contract.weeks.back().days.back();
    auto& Bar = Day.bars.back();
    
    // updating the day basics
    Day.dayHigh = std::max(Day.dayHigh, Bar.high);
    Day.dayLow = std::min(Day.dayLow, Bar.low);
    Day.dayClose = currentPrice;

    // week basic
    WEEK.weekHigh = std::max(WEEK.weekHigh, Day.dayHigh);
    
    WEEK.weekLow = std::min(WEEK.weekLow, Day.dayLow);


    

    // calculating the indicators 
    calculateDayVWAP(contract);
    calculateWeekVWAP(contract);
    calculateBBands(contract);
    calculateRSI(contract);
    calculateDayTPO(contract);
    calculateWeekTPO(contract);
    calculateDeltaZscore(contract);
    calculateCumDelta5barsSlope(contract);
    



    
    
}