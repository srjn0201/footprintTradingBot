#ifndef UPDATEFEATURES_H
#define UPDATEFEATURES_H



#include "../dataStructure.h"
#include <string>



void checkForSignal(Contract& contract);

void updateFootprint(Contract& contract, double currentPrice, int currentAskVolume, int currentBidVolume);
void updateTickSensitiveFeatures(Contract& contract, double currentPrice, int currentAskVolume, int currentBidVolume);
void updatePriceSensitiveFeatures(Contract& contract, double currentPrice, int currentAskVolume, int currentBidVolume);
void updateBarChangeSensitiveFeatures(Contract& contract, double currentPrice, int currentAskVolume, int currentBidVolume);
void updateDayChangeSensitiveFeatures(Contract& contract);
void updateWeekChangeSensitiveFeatures(Contract& contract);

void finalizeLastBar(Contract& contract, std::string datetime, double currentPrice, int currentAskVolume, int currentBidVolume);
void finalizeProcessingDay(Contract& contract);
void finalizeProcessingWeek(Contract& contract);
void finalizeContract(Contract& contract);




#endif // UPDATEFEATURES_H