#include "../dataStructure.h"




// price-TPO difference
    // price-VAH diff
    // price-VAL diff
    // price-POCDiff (current day poc)
    // price-POCDiff (previous day poc)
    // isPriceInVA
    // 
// price-indicator difference
    // price-VWAP diff
    // price-VWAP std diff
    // price-BBU diff
    // price-BBL diff
    // price-BBM diff

// S/R diff
    // price-prevDayHigh diff
    // price-previousDayLow diff
    // price-IBHigh diff
    // pirce-IBLow diff


void updatePriceSensitiveFeatures(Contract& contract, double currentPrice, int currentAskVolume, int currentBidVolume) {
    // Update footprint
    auto& DAY = contract.weeks.back().days.back();

    
    
}