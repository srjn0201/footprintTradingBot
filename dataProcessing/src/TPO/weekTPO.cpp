#include <map>
#include <algorithm>
#include <limits>
#include <iterator>
#include <iostream>
#include "dataStructure.h" // Assuming your structs are here

// Using the same type aliases
using PriceType = double;
using VolumeType = int64_t;

// 
//  * @brief Calculates the Volume Profile (POC, VAH, VAL, HVN) for a Week struct.
//  * @param contract The Contract object, used to access the current week.
//  * * This function aggregates all volume from all bars in all days of the
//  * current week to build a composite weekly profile.
//  

void calculateWeekTPO(Contract& contract) {
    // Get a reference to the current week
    auto& week = contract.weeks.back();

    // --- Step 1: Aggregate Volumes (O(N log U) aggregation) ---
    // N is now the total number of data points in the *entire week*
    std::map<PriceType, VolumeType> profileMap;
    VolumeType totalWeekVolume = 0;

    // Iterate through each day in the week
    for (const auto& day : week.days) {
        // Iterate through each bar in that day
        for (const auto& bar : day.bars) {
            // Iterate through each price level in that bar's footprint
            for (const auto& pair : bar.footprint.priceLevels) {
                PriceType price = pair.first;
                VolumeType volumeAtPrice = pair.second.bidVolume + pair.second.askVolume;
                
                profileMap[price] += volumeAtPrice;
                totalWeekVolume += volumeAtPrice;
            }
        }
    }

    // --- Handle edge case of an empty week ---
    if (profileMap.empty()) {
        week.poc = 0.0;
        week.vah = 0.0;
        week.val = 0.0;
        // week.lastHighVolumeNode = 0.0; // Your Week struct doesn't have this
        week.totalVolume = 0;
        return;
    }

    // --- Step 2: Find VPOC & Secondary HVN (O(U) scan) ---
    VolumeType maxVolume = 0;
    VolumeType secondMaxVolume = 0;
    PriceType pocPrice = 0.0;
    PriceType hvnPrice = 0.0; // Secondary POC
    
    std::map<PriceType, VolumeType>::iterator pocIterator = profileMap.begin();

    for (auto it = profileMap.begin(); it != profileMap.end(); ++it) {
        if (it->second > maxVolume) {
            secondMaxVolume = maxVolume;
            hvnPrice = pocPrice;
            
            maxVolume = it->second;
            pocPrice = it->first;
            pocIterator = it; 
        } else if (it->second > secondMaxVolume) {
            secondMaxVolume = it->second;
            hvnPrice = it->first;
        }
    }

    // --- Step 3: Calculate Value Area (O(U) scan from POC) ---
    const double valueAreaPercentage = 0.70;
    VolumeType targetVolume = static_cast<VolumeType>(totalWeekVolume * valueAreaPercentage);
    VolumeType currentVolume = maxVolume;
    
    auto vahIterator = pocIterator;
    auto valIterator = pocIterator;

    while (currentVolume < targetVolume) {
        auto itAbove = std::next(vahIterator, 1);
        VolumeType volumeAbove = (itAbove != profileMap.end()) 
                               ? itAbove->second 
                               : 0;
        
        auto itBelow = (valIterator != profileMap.begin()) 
                       ? std::prev(valIterator, 1) 
                       : profileMap.end(); 
        
        VolumeType volumeBelow = (itBelow != profileMap.end()) 
                               ? itBelow->second 
                               : 0;

        if (volumeAbove == 0 && volumeBelow == 0) {
            break;
        }

        if (volumeAbove > volumeBelow) {
            currentVolume += volumeAbove;
            vahIterator = itAbove;
        } else {
            currentVolume += volumeBelow;
            valIterator = itBelow;
        }
    }

    // --- Step 4: Update the Week Struct ---
    week.poc = pocPrice;
    week.vah = vahIterator->first;
    week.val = valIterator->first;
    week.totalVolume = totalWeekVolume;

    week.lastHighVolumeNode = hvnPrice;
}