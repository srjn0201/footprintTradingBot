#include "dataStructure.h"
#include "database/database.h"
#include <stdexcept>
#include <cmath>



void initializeContract(Contract& contract, const std::string& database_path, const std::string& table_name, const Date& startDate) {
    // Fetch the first tick data for the start date
    TickData tickData = fetchFirstTick(database_path, table_name, startDate);

    if (tickData.id == -1) {
        throw std::runtime_error("No data found for the given start date.");
    }

    double initialPrice = tickData.Price;


    // Initialize price-related fields in Bar
    Bar initialBar;
    initialBar.open = initialPrice;
    initialBar.close = initialPrice;
    initialBar.high = initialPrice;
    initialBar.low = initialPrice;
    initialBar.barPOCPrice = initialPrice;
    initialBar.barPOCVol = 0;
    initialBar.startTime = "-1";
    initialBar.endTime = "-1";
    initialBar.barTotalVolume = 0;
    initialBar.barDeltaChange = 0;
    initialBar.barHighDelta = 0;
    initialBar.barLowDelta = 0;
    initialBar.signal = 0;
    initialBar.signalID = -1;
    initialBar.signalStatus = false;
    initialBar.priceBBandLowerDiff = 0.0;
    initialBar.priceBBandUpperDiff = 0.0;

    // extra initialixation
    
    // Initialize price-related fields in Day
    Day initialDay;
    initialDay.vwap = initialPrice;
    initialDay.vwapUpperStdDev1 = initialPrice;
    initialDay.vwapLowerStdDev1 = initialPrice;
    initialDay.bbMiddle = initialPrice;
    initialDay.bbUpper = initialPrice;
    initialDay.bbLower = initialPrice;
    initialDay.poc = initialPrice;
    initialDay.vah = initialPrice;
    initialDay.val = initialPrice;
    initialDay.ibHigh = initialPrice;
    initialDay.ibLow = initialPrice;
    initialDay.dayHigh = initialPrice;
    initialDay.dayLow = initialPrice;
    initialDay.dayClose = initialPrice;
    initialDay.lastSwingHigh = initialPrice;
    initialDay.lastSwingLow = initialPrice;
    initialDay.lastHighVolumeNode = initialPrice;
    
    initialDay.prevAvgGain = std::nan("");
    initialDay.prevAvgLoss = std::nan("");

    // Add the initialized bar to the day
    initialDay.bars.push_back(initialBar);

    // Initialize price-related fields in Week
    Week initialWeek;
    initialWeek.vwap = initialPrice;
    initialWeek.poc = initialPrice;
    initialWeek.vah = initialPrice;
    initialWeek.val = initialPrice;
    initialWeek.weekHigh = initialPrice;
    initialWeek.weekLow = initialPrice;

    // Add the initialized day to the week
    initialWeek.days.push_back(initialDay);

    // Add the initialized week to the contract
    contract.weeks.push_back(initialWeek);

}