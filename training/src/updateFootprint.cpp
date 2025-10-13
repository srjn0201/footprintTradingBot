#include "../dataStructure.h"



void updateFootprint(Contract& contract, double price, double bidVolume, double askVolume) {
    // if price level exists, update volumes
    auto& FOOTPRINT = contract.weeks.back().days.back().bars.back().footprint;
    if (FOOTPRINT.priceLevels.find(price) != FOOTPRINT.priceLevels.end()){
        FOOTPRINT.priceLevels[price].bidVolume += bidVolume;
        FOOTPRINT.priceLevels[price].askVolume += askVolume;
    }
    else{
    // else create new price level and set volumes
    PriceLevel newLevel;
    newLevel.bidVolume = bidVolume;
    newLevel.askVolume = askVolume;
    newLevel.isBuyImbalance = false;
    newLevel.isSellImbalance = false;
    FOOTPRINT.priceLevels[price] = newLevel;
    }
    

    //calculate imbalance
    if (!FOOTPRINT.priceLevels.empty() && FOOTPRINT.priceLevels.size() >= 2 ) {
        //for buy imbalance
        // first check if the price is not the lowest entry to avoid runtime error because if it is lowest then we cont have anything at bid to calculate the imbalance
        if (!(FOOTPRINT.priceLevels.begin()->first == price)) {
            // calculating the ibmalance
            // also checking if the denominator is not zero to avoid runtime error
            double denominator_b = FOOTPRINT.priceLevels[price-.25].bidVolume;
            if (denominator_b == 0) {
                denominator_b = 1;
            } 
            if ((FOOTPRINT.priceLevels[price].askVolume / 
                denominator_b) >=2.9 ) {
                    FOOTPRINT.priceLevels[price].isBuyImbalance = true;
                }
        
        }
            //for sell imbalance
            // first check if the price is not the highest entry to avoid runtime error because if it
        if (!(FOOTPRINT.priceLevels.rbegin()->first == price)) {
            // calculating the ibmalance
            // also checking if the denominator is not zero to avoid runtime error
            double denominator_s = FOOTPRINT.priceLevels[price+.25].askVolume;
            if (denominator_s == 0) {
                denominator_s = 1;
            }
            if ((FOOTPRINT.priceLevels[price].bidVolume / 
                denominator_s) >=2.9 ) {
                FOOTPRINT.priceLevels[price].isSellImbalance = true;
            }
        }        
    }
}


