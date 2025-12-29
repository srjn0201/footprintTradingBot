#include "dataStructure.h"
#include "database/database.h"
#include <stdexcept>
#include <cmath>



void initializeNewDay(Contract& contract, double firstPrice, int dayOfWeek){
    
    auto& WEEK = contract.weeks.back();

    

    Day newDay;
    newDay.dayHigh = firstPrice;
    newDay.dayLow = firstPrice;
    newDay.dayClose = firstPrice;
    newDay.totalVolume = 0;
    newDay.dayOfTheWeek = std::to_string(dayOfWeek);
    newDay.vwap = firstPrice;
    newDay.poc = firstPrice;
    newDay.vah = firstPrice;
    newDay.val = firstPrice;
    newDay.dayHigh = firstPrice;
    newDay.dayLow = firstPrice;
    newDay.dayClose = firstPrice;
    newDay.cumulativeDelta = 0;
    newDay.lastSwingHigh = 0;
    newDay.lastSwingLow = 0;
    newDay.bbMiddle = firstPrice;
    newDay.bbUpper = firstPrice;
    newDay.bbLower = firstPrice;
    newDay.variance = 0.0;





    Bar newBar;
    newBar.startTime = "-1";
    newBar.endTime = "-1";
    newBar.open = firstPrice;
    newBar.close = firstPrice;
    newBar.high = firstPrice;
    newBar.low = firstPrice;
    newBar.barTotalVolume = 0;



    newDay.bars.push_back(newBar);
    WEEK.days.push_back(newDay);

    if (WEEK.weekLow == 0.0){WEEK.weekLow = firstPrice;}
    




}