#ifndef FOOTPRINT_DATA_STRUCTURE_H
#define FOOTPRINT_DATA_STRUCTURE_H

#include <map>
#include <vector>
#include <string>
#include <ctime>


// Volume information at a specific price level and time whitin the bar
// updation will be at each tick iteration
struct PriceLevel {
    double bidVolume;
    double askVolume;
    bool isBuyImbalance; // true for buy imbalance
    bool isSellImbalance; // true for sell imbalance
};


// footprint data for the bar
// updtion will be at each tick iteration
struct Footprint {
    std::map<double, PriceLevel> priceLevels;  // Time-based volume information
    int imbalanceCount; // Number of imbalances detected in the bar
    int delta; // Net volume delta (buy volume - sell volume)
    int barDeltaChange; // Change in delta from the previous bar
    int barHighDelta; // delta of the top two price levels in the bar
    int barLowDelta; // delta of the bottom two price levels in the bar
    double barPOC; // Point of Control price level in the bar
    
};


// bar data structure stores the data that can a single bar will hold within the whole cahrt data
// updation will occur at different frequency based on the calculations
struct Bar {
    Footprint footprint; // Footprint data of the bar

// bar's basic OHLCV data and will be calculated at each tick iteration
    time_t startTime;          // Start time of the bar
    time_t endTime;            // End time of the bar
    double open;         // Opening price
    double close;        // Closing price
    double high;         // Highest price
    double low;          // Lowest price
    double barTotalVolume;       // Total traded volume
    
// signal related data for the bar, will be updated once when signal is generated
// if signal will checked with many fixed set of rules(like minVolume, imbalance threshhold and etc) at each tick only if the signalStatus is false
// only one signal per candle
    int signal;       // Trading signal for the bar (2 for buy, 1 for sell, 0 for neutral)
    int signalID;     // Unique identifier for the signal, is the index of the tick when the signal was generated from the raw data
    bool signalStatus; // true if the signal is alresdy generated for the bar, false otherwise
    
// price-indicators difference value for the bar, will be calculated only if price changed
    double priceCurrentDayVwapDiff;      // Difference between last price and current day underdeveloping VWAP
    double priceCurrentDayVwapUpperStdDevDiff; // Difference between last price and Day VWAP + 1 Std Dev
    double priceCurrentDayVwapLowerStdDevDiff; // Difference between last price and Day VWAP - 1 Std Dev
    double pricePreviousDayVwapDiff;     // Difference between last price and previous day VWAP
    double priceWeeklyVwapDiff;          // Difference between last price and weekly VWAP
    double priceBBandUpperDiff;        // Difference between last price and upper Bollinger Band
    double priceBBandLowerDiff;        // Difference between last price and lower Bollinger Band
    double PriceBBandMiddleDiff;       // Difference between last price and middle Bollinger Band

// price-TPO difference value, will be calculated only if price changed
    double priceCurrDayVAHDiff;    // Difference between last price and Value Area High of the current underdeveloping profile
    double priceCurrDayVALDiff;     // Difference between last price and Value Area Low of the current underdeveloping profile
    double pricePrevDayPOCDiff;          // Difference between last price and Point of Control of the previous day profile
    double pricePrevDayVAHDiff;   // Difference between last price and previous day Value Area High
    double pricePrevDayVALDiff;    // Difference between last price and previous day Value Area

// price S/R difference value, will be calculated only if price changed
    double priceIBHighDiff;       // Difference between last price and Initial Balance High
    double priceIBLowDiff;        // Difference between last price and Initial Balance Low
    double pricePrevDayHighDiff;   // Difference between last price and previous day high
    double pricePrevDayLowDiff;    // Difference between last price and previous day low
    double pricePrevDayCloseDiff;  // Difference between last price and previous day close
    double priceWeekHighDiff;      // Difference between last price and week high
    double priceWeekLowDiff;       // Difference between last price and week low

    double priceLastSwingHighDiff; // Difference between last price and last swing high
    double priceLastSwingLowDiff;  // Difference between last price and last swing low
    double priceLastHVNDiff;      // Difference between last price and last high volume node


};

struct Day {
    std::vector<Bar> bars;
    time_t dayOfTheWeek; 

// calculated on new bar creation
    double vwap; // VWAP for the day
    double vwapUpperStdDev; // Standard deviation of VWAP for the day
    double vwapLowerStdDev; // Standard deviation of VWAP for the day
    double bbMiddle; // Middle Bollinger Band for the day
    double bbUpper; // Upper Bollinger Band for the day
    double bbLower; // Lower Bollinger Band for the day
    double BBandWidth;                     // Width of the Bollinger Bands
    double rsi; // Relative Strength Index for the day
    double vah; // Value Area High for the day
    double val; // Value Area Low for the day

//calculated once 
    double ibHigh; // Initial Balance High for the day
    double ibLow; // Initial Balance Low for the day

    double dayHigh; // Highest price of the day calculated at the end of the day
    double dayLow; // Lowest price of the day calculated at the end of the day
    double dayClose; // Closing price of the day calculated at the end of the day

    double totalVolume; // Total volume traded during the day
    double cumulativeDelta; // Cumulative delta for the day
    double lastSwingHigh; // Last swing high price of the day calculated at bar update
    double lastSwingLow; // Last swing low price of the day calculated at bar update
    double lastHighVolumeNode; // Last high volume node price of the day calculated at bar update
    double poc; // Point of Control for the day calculated at the end of the day


};

struct Week {
// calculated at the end of the week
    std::vector<Day> days;
    time_t weekOfTheContract; // Start date of the week
    double vwap; // VWAP for the week
    double poc; // Point of Control for the week
    double vah; // Value Area High for the week
    double val; // Value Area Low for the week
    double weekHigh; // Highest price of the week
    double weekLow; // Lowest price of the week
};


struct Contract{
    std::vector<Week> weeks;
    std::string contractName; // Name of the futures contract
    
};



#endif // FOOTPRINT_DATA_STRUCTURE_H