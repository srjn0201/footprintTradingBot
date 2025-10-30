#include "../dataStructure.h"



void updateDayChangeSensitiveFeatures(Contract& contract) {
    // Update day change sensitive features
    auto& DAY = contract.weeks.back().days.back();

    DAY.dayHigh = 0.0;
    DAY.dayLow = 0.0;
    DAY.dayClose = 0.0;
    DAY.totalVolume = 0.0;
    DAY.cumulativeDelta = 0.0;
    DAY.lastSwingHigh = 0.0;
    DAY.lastSwingLow = 0.0;
    DAY.lastHighVolumeNode = 0.0;
    DAY.ibHigh = 0.0;
    DAY.ibLow = 0.0;
    DAY.vwap = 0.0;
    DAY.vwapUpperStdDev1 = 0.0;
    DAY.vwapLowerStdDev1 = 0.0;
    DAY.bbMiddle = 0.0;
    DAY.bbUpper = 0.0;
    DAY.bbLower = 0.0;
    DAY.BBandWidth = 0.0;
    DAY.rsi = 0.0;
    DAY.poc = 0.0;
    DAY.vah = 0.0;
    DAY.val = 0.0;
    DAY.deltaZscore20bars = 0.0;
    DAY.cumDelta5barSlope = 0.0;
    DAY.priceCumDeltaDivergence5bar = 0.0;
    DAY.priceCumDeltaDivergence10bar = 0.0;
    DAY.interactionReversal20bar = 0.0;
    
    
}