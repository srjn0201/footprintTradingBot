#include "../dataStructure.h"




void finalizeProcessingWeek(Contract& contract) {
    Bar newBar;
    Day newDay;
    Week newWeek;

    newBar.startTime = "-1";
    newBar.endTime = "-1";
    newBar.open = 0;
    newBar.close = 0;
    newBar.high = 0;
    newBar.low = 0;
    newBar.barTotalVolume = 0;


    newWeek.weekOfTheContract = "-1";
    newWeek.vwap = 0.0;
    newWeek.poc = 0.0;
    newWeek.vah = 0.0;
    newWeek.val = 0.0;
    newWeek.weekHigh = 0.0;

    contract.weeks.push_back(newWeek);
    contract.weeks.back().days.push_back(newDay);
    contract.weeks.back().days.back().bars.push_back(newBar);
    
    
}