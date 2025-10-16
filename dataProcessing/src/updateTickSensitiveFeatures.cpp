#include "../dataStructure.h"



// feature that are sensitive to every tick update
// signal and its derivatives will be checked seperately
    // basic bar features -----------------------------------------------
    // open
    // high
    // low
    // close
    // barTotalVolume
    // startTime
    // endTime

    // footprint features -----------------------------------------------
    // imbalanceCount
    // delta
    // barDeltaHigh
    // barDeltaLow

    //TPO features ------------------------------------------------
    // barPOC
    // vah
    // val


void updateTickSensitiveFeatures(Contract& contract, double currentPrice, int currentAskVolume, int currentBidVolume) {
    // Update footprint
    auto& BAR = contract.weeks.back().days.back().bars.back();
    
}