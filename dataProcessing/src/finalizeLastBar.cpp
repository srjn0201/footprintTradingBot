#include "../dataStructure.h"

// this function will do the following
    // first of all add new bar in the bars vector of the current day
        // initialize all the features of the new bar

void finalizeLastBar(Contract& contract, std::string currentTime, double currentPrice, int currentAskVolume, int currentBidVolume) {
    auto& DAY = contract.weeks.back().days.back();

    // UPDATE THE END TIME OF THE LAST BAR
    DAY.bars.back().endTime = currentTime;

    // first add new bar at the end of the bars vector
    //but first initializing values
    Bar newBar;
    newBar.startTime = currentTime;
    newBar.open = currentPrice;
    newBar.high = currentPrice;
    newBar.low = currentPrice;
    newBar.close = currentPrice;
    newBar.barTotalVolume = currentAskVolume + currentBidVolume;
    newBar.startTime = currentTime;
    newBar.endTime = "-1";
    newBar.barDeltaChange = 0;
    newBar.barHighDelta = 0;
    newBar.barLowDelta = 0;
    newBar.signal = 0;
    newBar.signalID = -1;
    newBar.signalStatus = false;
    newBar.priceBBandLowerDiff = 0.0;
    newBar.priceBBandUpperDiff = 0.0;

    PriceLevel newPriceLevel;
    Footprint& newFootprint = newBar.footprint;
    newPriceLevel.bidVolume = currentBidVolume;
    newPriceLevel.askVolume = currentAskVolume;
    newPriceLevel.isBuyImbalance = false;
    newPriceLevel.isSellImbalance = false;
    newFootprint.priceLevels[currentPrice] = newPriceLevel;

    //addding new bar
    
    DAY.bars.push_back(newBar);
    DAY.bars.back().footprint.priceLevels[currentPrice] = newPriceLevel;


}