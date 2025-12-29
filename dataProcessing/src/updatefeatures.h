#ifndef UPDATEFEATURES_H
#define UPDATEFEATURES_H



#include "../dataStructure.h"
#include <string>
#include <utility>



void checkForSignal(Contract& contract);

std::pair<int, int> updateFootprint(Contract& contract, double imbalanceThreshhold, double currentPrice, int currentAskVolume, int currentBidVolume);
void updateTickSensitiveFeatures(Contract& contract, double currentPrice, int currentAskVolume, int currentBidVolume, std::pair<int, int> imbalanceChange);
void updatePriceSensitiveFeatures(Contract& contract, double currentPrice, int currentAskVolume, int currentBidVolume);
void updateBarChangeSensitiveFeatures(Contract& contract, double currentPrice, int currentAskVolume, int currentBidVolume);
void updateDayChangeSensitiveFeatures(Contract& contract);
void updateWeekChangeSensitiveFeatures(Contract& contract);

void initializeNewBar(Contract& contract, std::string datetime, double currentPrice, int currentAskVolume, int currentBidVolume);
void finalizeProcessingDay(Contract& contract);
void initializeWeek(Contract& contract);
void finalizeContract(Contract& contract);




#endif // UPDATEFEATURES_H