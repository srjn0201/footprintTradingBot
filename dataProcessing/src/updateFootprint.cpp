#include "../dataStructure.h"



void updateFootprint(Contract& contract, double imbalanceThreshhold, double price, int bidVolume, int askVolume) {
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
    // --- New Imbalance Calculation Logic ---
    if (FOOTPRINT.priceLevels.size() >= 2) {
        // Use safe iterators to determine if the current price is at the top or bottom
        bool isLowestPrice = (FOOTPRINT.priceLevels.begin()->first == price);
        bool isHighestPrice = (FOOTPRINT.priceLevels.rbegin()->first == price);

        // Case 1: Current price is the LOWEST in the footprint
        if (isLowestPrice) {
            // A) Calculate SELL imbalance at the current price (price)
            double denominator_s_current = FOOTPRINT.priceLevels[price + 0.25].askVolume;
            if (denominator_s_current == 0) { denominator_s_current = 1; }
            if ((FOOTPRINT.priceLevels[price].bidVolume / denominator_s_current) >= imbalanceThreshhold) {
                FOOTPRINT.priceLevels[price].isSellImbalance = true;
            } else {
                FOOTPRINT.priceLevels[price].isSellImbalance = false;
            }

            // B) Calculate BUY imbalance at the next price level (price + 0.25)
            double denominator_b_next = FOOTPRINT.priceLevels[price].bidVolume;
            if (denominator_b_next == 0) { denominator_b_next = 1; }
            if ((FOOTPRINT.priceLevels[price + 0.25].askVolume / denominator_b_next) >= imbalanceThreshhold) {
                FOOTPRINT.priceLevels[price + 0.25].isBuyImbalance = true;
            } else {
                FOOTPRINT.priceLevels[price + 0.25].isBuyImbalance = false;
            }
        }
        // Case 2: Current price is the HIGHEST in the footprint
        else if (isHighestPrice) {
            // A) Calculate BUY imbalance at the current price (price)
            double denominator_b_current = FOOTPRINT.priceLevels[price - 0.25].bidVolume;
            if (denominator_b_current == 0) { denominator_b_current = 1; }
            if ((FOOTPRINT.priceLevels[price].askVolume / denominator_b_current) >= imbalanceThreshhold) {
                FOOTPRINT.priceLevels[price].isBuyImbalance = true;
            } else {
                FOOTPRINT.priceLevels[price].isBuyImbalance = false;
            }

            // B) Calculate SELL imbalance at the previous price level (price - 0.25)
            double denominator_s_prev = FOOTPRINT.priceLevels[price].askVolume;
            if (denominator_s_prev == 0) { denominator_s_prev = 1; }
            if ((FOOTPRINT.priceLevels[price - 0.25].bidVolume / denominator_s_prev) >= imbalanceThreshhold) {
                FOOTPRINT.priceLevels[price - 0.25].isSellImbalance = true;
            } else {
                FOOTPRINT.priceLevels[price - 0.25].isSellImbalance = false;
            }
        }
        // Case 3: Current price is in the MIDDLE of the footprint
        else {
            // A) Calculate BUY imbalance at the current price (price)
            double denominator_b_current = FOOTPRINT.priceLevels[price - 0.25].bidVolume;
            if (denominator_b_current == 0) { denominator_b_current = 1; }
            if ((FOOTPRINT.priceLevels[price].askVolume / denominator_b_current) >= imbalanceThreshhold) {
                FOOTPRINT.priceLevels[price].isBuyImbalance = true;
            } else {
                FOOTPRINT.priceLevels[price].isBuyImbalance = false;
            }

            // B) Calculate SELL imbalance at the current price (price)
            double denominator_s_current = FOOTPRINT.priceLevels[price + 0.25].askVolume;
            if (denominator_s_current == 0) { denominator_s_current = 1; }
            if ((FOOTPRINT.priceLevels[price].bidVolume / denominator_s_current) >= imbalanceThreshhold) {
                FOOTPRINT.priceLevels[price].isSellImbalance = true;
            } else {
                FOOTPRINT.priceLevels[price].isSellImbalance = false;
            }

            // C) Calculate BUY imbalance at the next price level (price + 0.25)
            double denominator_b_next = FOOTPRINT.priceLevels[price].bidVolume;
            if (denominator_b_next == 0) { denominator_b_next = 1; }
            if ((FOOTPRINT.priceLevels[price + 0.25].askVolume / denominator_b_next) >= imbalanceThreshhold) {
                FOOTPRINT.priceLevels[price + 0.25].isBuyImbalance = true;
            } else {
                FOOTPRINT.priceLevels[price + 0.25].isBuyImbalance = false;
            }

            // D) Calculate SELL imbalance at the previous price level (price - 0.25)
            double denominator_s_prev = FOOTPRINT.priceLevels[price].askVolume;
            if (denominator_s_prev == 0) { denominator_s_prev = 1; }
            if ((FOOTPRINT.priceLevels[price - 0.25].bidVolume / denominator_s_prev) >= imbalanceThreshhold) {
                FOOTPRINT.priceLevels[price - 0.25].isSellImbalance = true;
            } else {
                FOOTPRINT.priceLevels[price - 0.25].isSellImbalance = false;
            }
        }
    }
}