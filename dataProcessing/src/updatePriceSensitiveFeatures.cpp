#include "../dataStructure.h"
#include "../src/indicators/indicators.h"



// price-TPO difference
    // price-VAH diff (current day)
    // price-VAH diff (prev day)

    // price-VAL diff(current day)
    // price-VAL diff(prev day)

    // price-POCDiff (current day poc)
    // price-POCDiff (previous day poc)
    // isPriceInVA
    // 
// price-indicator difference
    // price-VWAP diff (daily and weekly)
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
    auto& currentDAY = contract.weeks.back().days.back();
    auto& prevDAY = contract.weeks.back().days[contract.weeks.back().days.size() - 2];
    auto& currentBAR = currentDAY.bars.back();

// price-indicator diference
    //VWAP
    currentBAR.priceCurrentDayVwapDiff = currentPrice - currentDAY.vwap;
    currentBAR.priceCurrentDayVwapLowerStdDev1Diff = currentPrice - currentDAY.vwapLowerStdDev1;
    currentBAR.priceCurrentDayVwapUpperStdDev1Diff = currentPrice - currentDAY.vwapUpperStdDev1;
    currentBAR.priceCurrentDayVwapLowerStdDev2Diff = currentPrice - currentDAY.vwapLowerStdDev2;
    currentBAR.priceCurrentDayVwapUpperStdDev2Diff = currentPrice - currentDAY.vwapUpperStdDev2;
    currentBAR.pricePreviousDayVwapDiff = currentPrice - prevDAY.vwap;
    currentBAR.priceWeeklyVwapDiff = currentPrice - contract.weeks.back().vwap;
    currentBAR.priceWeeklyVwapLowerStdDev1Diff = currentPrice - contract.weeks.back().vwapLowerStdDev1;
    currentBAR.priceWeeklyVwapUpperStdDev1Diff = currentPrice - contract.weeks.back().vwapUpperStdDev1;

    //BBands
    currentBAR.priceBBandUpperDiff = currentPrice - currentDAY.bbUpper;
    currentBAR.priceBBandLowerDiff = currentPrice - currentDAY.bbLower;
    currentBAR.PriceBBandMiddleDiff = currentPrice - currentDAY.bbMiddle;

    //TPO(day)
    currentBAR.priceCurrDayVAHDiff = currentPrice - currentDAY.vah;
    currentBAR.priceCurrDayVALDiff = currentPrice - currentDAY.val;
    currentBAR.pricePrevDayVAHDiff = currentPrice - prevDAY.vah;
    currentBAR.pricePrevDayVALDiff = currentPrice - prevDAY.val;
    currentBAR.pricePrevDayPOCDiff = currentPrice - prevDAY.poc;
    if (currentPrice > currentDAY.val && currentPrice < currentDAY.vah){ currentBAR.isPriceInCurrentDayVA = true;}
    else {currentBAR.isPriceInCurrentDayVA= false;}
    if (currentPrice > prevDAY.val && currentPrice < prevDAY.vah){ currentBAR.isPriceInPrevDayVA = true;}
    else {currentBAR.isPriceInPrevDayVA= false;}
    //TPO(week)
    // currentBAR.wee

    // S/R
    currentBAR.pricePrevDayHighDiff = currentPrice - prevDAY.dayHigh;
    currentBAR.pricePrevDayLowDiff = currentPrice - prevDAY.dayLow;
    currentBAR.pricePrevDayCloseDiff = currentPrice - prevDAY.dayClose;
    currentBAR.priceCurrentWeekHighDiff = currentPrice - contract.weeks.back().weekHigh;
    currentBAR.priceCurrentWeekLowDiff = currentPrice - contract.weeks.back().weekLow;
    currentBAR.pricePrevWeekHighDiff = currentPrice - contract.weeks[contract.weeks.size() - 2].weekHigh;
    currentBAR.pricePrevWeekLowDiff = currentPrice - contract.weeks[contract.weeks.size() - 2].weekLow;
// to be calculated
    currentBAR.priceIBHighDiff = currentPrice - currentDAY.ibHigh;
    currentBAR.priceIBLowDiff = currentPrice - currentDAY.ibLow;
    // currentBAR.priceLastSwingHighDiff = currentPrice - currentDAY.lastSwingHigh;
    // currentBAR.priceLastSwingLowDiff = currentPrice - currentDAY.lastSwingLow;
    // currentBAR.priceLastHVNDiff = currentPrice - currentDAY.lastHighVolumeNode;


// delta divergence
    currentDAY.priceCumDeltaDivergence5bar = calculatePriceCumDeltaDivergence(contract, 5);
    currentDAY.priceCumDeltaDivergence10bar = calculatePriceCumDeltaDivergence(contract, 10);

// interaction reversal
    calculateInteractionReversal(contract);
    
    
}