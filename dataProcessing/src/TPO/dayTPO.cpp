#include <map>           // Using std::map for aggregation
#include <algorithm>     // For std::sort
#include <limits>        // For std::numeric_limits
#include <iterator>      // For std::next/prev

#include "dataStructure.h"


//  * @brief Calculates the Volume Profile (POC, VAH, VAL, HVN) for a Day struct.
//  * @param day The Day object to analyze and update (passed by reference).
//  *
//  * NOTE: This version uses `double` for price keys as requested. To avoid
//  * floating-point precision errors, it MUST use `std::map` for aggregation,
//  * which is reliable but slower (O(N log U)) than the `int64_t` version.
using PriceType = double;
using VolumeType = int64_t;

void calculateDayTPO(Contract& contract) {
    auto& day = contract.weeks.back().days.back();

    // --- Step 1: Aggregate Volumes (O(N log U) aggregation) ---
    // We MUST use std::map. Using std::unordered_map with double keys will fail.
    std::map<PriceType, VolumeType> profileMap;
    VolumeType totalDayVolume = 0;

    for (const auto& bar : day.bars) {
        for (const auto& pair : bar.footprint.priceLevels) {
            PriceType price = pair.first;
            VolumeType volumeAtPrice = pair.second.bidVolume + pair.second.askVolume;
            
            profileMap[price] += volumeAtPrice;
            totalDayVolume += volumeAtPrice;
        }
    }

    // --- Handle edge case of an empty day ---
    if (profileMap.empty()) {
        day.poc = 0.0;
        day.vah = 0.0;
        day.val = 0.0;
        day.lastHighVolumeNode = 0.0;
        day.totalVolume = 0;
        return;
    }

    // --- Step 2: Find VPOC & Secondary HVN (O(U) scan) ---
    VolumeType maxVolume = 0;
    VolumeType secondMaxVolume = 0;
    PriceType pocPrice = 0.0;
    PriceType hvnPrice = 0.0; // lastHighVolumeNode (secondary POC)
    
    // We need to find the map iterator for the POC
    std::map<PriceType, VolumeType>::iterator pocIterator = profileMap.begin();

    for (auto it = profileMap.begin(); it != profileMap.end(); ++it) {
        if (it->second > maxVolume) {
            // Demote old max to second max
            secondMaxVolume = maxVolume;
            hvnPrice = pocPrice;
            
            // Promote new max
            maxVolume = it->second;
            pocPrice = it->first;
            pocIterator = it; // Store the iterator to the POC
        } else if (it->second > secondMaxVolume) {
            // New second max
            secondMaxVolume = it->second;
            hvnPrice = it->first;
        }
    }

    // --- Step 3: Calculate Value Area (O(U) scan from POC) ---
    const double valueAreaPercentage = 0.70;
    VolumeType targetVolume = static_cast<VolumeType>(totalDayVolume * valueAreaPercentage);
    VolumeType currentVolume = maxVolume;
    
    // Start boundaries at the POC
    auto vahIterator = pocIterator;
    auto valIterator = pocIterator;

    while (currentVolume < targetVolume) {
        // Get iterator one tick above the current VAH
        auto itAbove = std::next(vahIterator, 1);
        VolumeType volumeAbove = (itAbove != profileMap.end()) 
                               ? itAbove->second 
                               : 0;
        
        // Get iterator one tick below the current VAL
        auto itBelow = (valIterator != profileMap.begin()) 
                       ? std::prev(valIterator, 1) 
                       : profileMap.end(); // Use .end() as a 'null' flag
        
        VolumeType volumeBelow = (itBelow != profileMap.end()) 
                               ? itBelow->second 
                               : 0;

        // If we've reached the ends of the profile, stop
        if (volumeAbove == 0 && volumeBelow == 0) {
            break;
        }

        // Add the larger of the two surrounding volumes
        if (volumeAbove > volumeBelow) {
            currentVolume += volumeAbove;
            vahIterator = itAbove; // Expand Value Area high
        } else {
            currentVolume += volumeBelow;
            valIterator = itBelow; // Expand Value Area low
        }
    }

    // --- Step 4: Update the Day Struct ---
    day.poc = pocPrice;
    day.lastHighVolumeNode = hvnPrice;
    day.vah = vahIterator->first;
    day.val = valIterator->first;
    day.totalVolume = totalDayVolume;
}