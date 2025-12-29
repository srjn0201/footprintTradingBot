#include "../dataStructure.h"



// feature that are sensitive to every tick update
// signal and its derivatives will be checked seperately
  // basic bar features -----------------------------------------------
    // barTotalVolume

  // footprint features -----------------------------------------------
    // buyImbalanceCount
    // sellImbalanceCount
    // barDelta
    // barDeltaHigh
    // barDeltaLow
    // barDeltaChange

    //TPO features ------------------------------------------------
    // barPOC


void updateTickSensitiveFeatures(Contract& contract, double currentPrice, int currentAskVolume, int currentBidVolume, std::pair<int, int> imbalanceChange) {
    // Update footprint
    auto& BAR = contract.weeks.back().days.back().bars.back();

    // basic
    BAR.barTotalVolume += (currentAskVolume + currentBidVolume);

    //footprint matrics
    BAR.delta += (currentAskVolume - currentBidVolume);
    // BAR.barDeltaChange = 
    BAR.buyImbalanceCount += imbalanceChange.first;
    BAR.sellImbalanceCount += imbalanceChange.second;


        // updating poc price and volume 
    const auto& PRICELEVEL = BAR.footprint.priceLevels.at(currentPrice);
    if (PRICELEVEL.volumeAtPrice > BAR.barPOCVol){
        BAR.barPOCPrice = currentPrice;
        BAR.barPOCVol = PRICELEVEL.volumeAtPrice;
    
    }



    // std::map is sorted, so rbegin() and begin() give us fast access to extremes.
    auto& priceLevels = BAR.footprint.priceLevels;
    if (!priceLevels.empty()) {
        // Bar High Delta (top two price levels)
        auto r_it = priceLevels.rbegin(); // Highest price
        int64_t high_delta = r_it->second.deltaAtPrice;
        if (std::next(r_it)!= priceLevels.rend()) { // Check if a second level exists
            r_it++; // Move to the second highest price
            high_delta += r_it->second.deltaAtPrice;
        }
        BAR.barHighDelta = high_delta;

        // Bar Low Delta (bottom two price levels)
        auto it = priceLevels.begin(); // Lowest price
        int64_t low_delta = it->second.deltaAtPrice;
        if (std::next(it)!= priceLevels.end()) { // Check if a second level exists
            it++; // Move to the second lowest price
            low_delta += it->second.deltaAtPrice;
        }
        BAR.barLowDelta = low_delta;
    }
    
    // updating cumulative delta
    contract.weeks.back().days.back().cumulativeDelta += (currentAskVolume - currentBidVolume);


    //updating delta change
    if (contract.weeks.back().days.back().bars.size()<2) {BAR.barDeltaChange = 0;} else {
        BAR.barDeltaChange = BAR.delta - contract.weeks.back().days.back().bars[contract.weeks.back().days.back().bars.size() - 2].delta;
      }
        

    
}