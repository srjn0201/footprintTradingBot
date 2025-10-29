#include "../dataStructure.h"



std::pair<int, int> updateFootprint(Contract& contract, double imbalanceThreshhold, double price, int bidVolume, int askVolume) {
    // if price level exists, update volumes
    auto& FOOTPRINT = contract.weeks.back().days.back().bars.back().footprint;
    if (FOOTPRINT.priceLevels.find(price) != FOOTPRINT.priceLevels.end()){
        FOOTPRINT.priceLevels[price].bidVolume += bidVolume;
        FOOTPRINT.priceLevels[price].askVolume += askVolume;
        FOOTPRINT.priceLevels[price].volumeAtPrice += (bidVolume + askVolume);
        FOOTPRINT.priceLevels[price].deltaAtPrice += (askVolume - bidVolume);
        
    }
    else{
    // else create new price level and set volumes
    PriceLevel newLevel;
    newLevel.bidVolume = bidVolume;
    newLevel.askVolume = askVolume;
    newLevel.isBuyImbalance = false;
    newLevel.isSellImbalance = false;
    newLevel.volumeAtPrice = (bidVolume + askVolume);
    newLevel.deltaAtPrice = (askVolume - bidVolume);
    FOOTPRINT.priceLevels[price] = newLevel;
    }
    


    if (FOOTPRINT.priceLevels.size() < 2) {
        return {0, 0}; // Not enough levels to have an imbalance, no change.
    }

    // --- 2. Track Imbalance Changes ---
    // A single tick can affect the imbalance status of up to three price levels:
    // - The Buy Imbalance at the level ABOVE the current price.
    // - The Buy and Sell Imbalance at the CURRENT price.
    // - The Sell Imbalance at the level BELOW the current price.
    // We must check all of them and calculate the net change.

    int buy_imb_change = 0;
    int sell_imb_change = 0;

    // --- Helper Lambda to check and update an imbalance flag ---
    // This avoids code duplication and makes the logic clearer.
    auto check_and_update =[](bool& flag, int64_t aggressive_vol, int64_t passive_vol, double ratio) {
        bool old_state = flag;
        bool new_state = false;
        if (passive_vol == 0 && aggressive_vol > 0) {
            new_state = true;
        } else if (passive_vol > 0 && (static_cast<double>(aggressive_vol) / passive_vol) >= ratio) {
            new_state = true;
        }
        flag = new_state;
        return (new_state - old_state); // Returns +1 if created, -1 if removed, 0 if unchanged
    };



    //calculate imbalance
    // --- New Imbalance Calculation Logic ---
    if (FOOTPRINT.priceLevels.size() >= 2) {
        // Use safe iterators to determine if the current price is at the top or bottom
        bool isLowestPrice = (FOOTPRINT.priceLevels.begin()->first == price);
        bool isHighestPrice = (FOOTPRINT.priceLevels.rbegin()->first == price);

        // Case 1: Current price is the LOWEST in the footprint
        if (isLowestPrice) {
            // A) Calculate SELL imbalance at the current price (price)
            sell_imb_change += check_and_update(
            FOOTPRINT.priceLevels[price].isSellImbalance, // imbalance flag
            FOOTPRINT.priceLevels[price].bidVolume, // aggressor
            FOOTPRINT.priceLevels[price + 0.25].askVolume,
            imbalanceThreshhold );

            // B) Calculate BUY imbalance at the next price level (price + 0.25)
            buy_imb_change += check_and_update(
            FOOTPRINT.priceLevels[price+0.25].isBuyImbalance, // imbalance flag
            FOOTPRINT.priceLevels[price+0.25].askVolume, // The value that just changed
            FOOTPRINT.priceLevels[price].bidVolume,
            imbalanceThreshhold );
        }
        // Case 2: Current price is the HIGHEST in the footprint
        else if (isHighestPrice) {
            // A) Calculate BUY imbalance at the current price (price)
            buy_imb_change += check_and_update(
            FOOTPRINT.priceLevels[price].isBuyImbalance, // imbalance flag
            FOOTPRINT.priceLevels[price].askVolume, // aggressor
            FOOTPRINT.priceLevels[price-0.25].bidVolume,
            imbalanceThreshhold );

            // B) Calculate SELL imbalance at the previous price level (price - 0.25)
            sell_imb_change += check_and_update(
            FOOTPRINT.priceLevels[price-0.25].isSellImbalance, // imbalance flag
            FOOTPRINT.priceLevels[price-0.25].bidVolume, // aggressor
            FOOTPRINT.priceLevels[price].askVolume,
            imbalanceThreshhold );
        }
        // Case 3: Current price is in the MIDDLE of the footprint
        else {
            // A) Calculate BUY imbalance at the current price (price)
            buy_imb_change += check_and_update(
            FOOTPRINT.priceLevels[price].isBuyImbalance, // imbalance flag
            FOOTPRINT.priceLevels[price].askVolume, // aggressor
            FOOTPRINT.priceLevels[price-0.25].bidVolume,
            imbalanceThreshhold );

            // B) Calculate SELL imbalance at the current price (price)
            sell_imb_change += check_and_update(
            FOOTPRINT.priceLevels[price].isSellImbalance, // imbalance flag
            FOOTPRINT.priceLevels[price].bidVolume, // aggressor
            FOOTPRINT.priceLevels[price + 0.25].askVolume,
            imbalanceThreshhold );

            // C) Calculate BUY imbalance at the next price level (price + 0.25)
            buy_imb_change += check_and_update(
            FOOTPRINT.priceLevels[price+0.25].isBuyImbalance, // imbalance flag
            FOOTPRINT.priceLevels[price+0.25].askVolume, // The value that just changed
            FOOTPRINT.priceLevels[price].bidVolume,
            imbalanceThreshhold );

            // D) Calculate SELL imbalance at the previous price level (price - 0.25)
            sell_imb_change += check_and_update(
            FOOTPRINT.priceLevels[price-0.25].isSellImbalance, // imbalance flag
            FOOTPRINT.priceLevels[price-0.25].bidVolume, // aggressor
            FOOTPRINT.priceLevels[price].askVolume,
            imbalanceThreshhold );
        }
    }

    // --- 4. Return the net changes ---
    return {buy_imb_change, sell_imb_change};
}