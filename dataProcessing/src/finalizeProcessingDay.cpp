#include "../dataStructure.h"



void finalizeProcessingDay(Contract& contract) {
    auto& WEEK = contract.weeks.back();

    

    Day newDay;
    Bar newBar;
    newBar.startTime = "-1";
    newBar.endTime = "-1";
    newBar.open = 0;
    newBar.close = 0;
    newBar.high = 0;
    newBar.low = 0;
    newBar.barTotalVolume = 0;


    newDay.bars.push_back(newBar);
    WEEK.days.push_back(newDay);


}