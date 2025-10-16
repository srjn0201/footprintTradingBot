#ifndef FOOTPRINT_DATA_STRUCTURE_H
#define FOOTPRINT_DATA_STRUCTURE_H

#include <map>
#include <vector>
#include <string>
#include <ctime>
#include <array>



// Define a simple Date type and week container
struct Date {
	int y, m, d;
};

struct DayEntry {
	int dayNumber;  // 1 (Monday) through 7 (Sunday)
	Date date;
};

struct weekVector {
    int weekNumber;
    std::vector<DayEntry> days;
    Date startDate;
    Date endDate;
};

// Volume information at a specific price level and time whitin the bar
// updation will be at each tick iteration
struct PriceLevel {
    double bidVolume = 0.0;
    double askVolume = 0.0;
    bool isBuyImbalance = false; // true for buy imbalance
    bool isSellImbalance = false; // true for sell imbalance
};


// footprint data for the bar
// updtion will be at each tick iteration
struct Footprint {
    std::map<double, PriceLevel> priceLevels;  // Time-based volume information
};



// bar data structure stores the data that can a single bar will hold within the whole cahrt data
// updation will occur at different frequency based on the calculations
struct Bar {
// calculated at each tick iteration
    Footprint footprint; // Footprint data of the bar
    int imbalanceCount = 0; // Number of imbalances detected in the bar
    int delta = 0; // Net volume delta (buy volume - sell volume)
    int barDeltaChange = 0; // Change in delta from the previous bar
    int barHighDelta = 0; // delta of the top two price levels in the bar
    int barLowDelta = 0; // delta of the bottom two price levels in the bar
    double barPOC = 0.0; // Point of Control price level in the bar
    
    // bar's basic OHLCV data and will be calculated at each tick iteration
    std::string startTime = "-1";          // Start time of the bar
    std::string endTime = "-1";            // End time of the bar
    double open = 0.0;         // Opening price
    double close = 0.0;        // Closing price
    double high = 0.0;         // Highest price
    double low = 0.0;          // Lowest price
    int barTotalVolume = 0;       // Total traded volume
    
// signal related data for the bar, will be updated once when signal is generated
// if signal will checked with many fixed set of rules(like minVolume, imbalance threshhold and etc) at each tick only if the signalStatus is false
// only one signal per candle
    int signal = 0;       // Trading signal for the bar (2 for buy, 1 for sell, 0 for neutral)
    int signalID = -1;     // Unique identifier for the signal, is the index of the tick when the signal was generated from the raw data, -1 if no signal
    bool signalStatus = false; // true if the signal is alresdy generated for the bar, false otherwise
    
// price-indicators difference value for the bar, will be calculated only if price changed
    double priceCurrentDayVwapDiff = 0.0;      // Difference between last price and current day underdeveloping VWAP
    double priceCurrentDayVwapUpperStdDevDiff = 0.0; // Difference between last price and Day VWAP + 1 Std Dev
    double priceCurrentDayVwapLowerStdDevDiff = 0.0; // Difference between last price and Day VWAP - 1 Std Dev
    double pricePreviousDayVwapDiff = 0.0;     // Difference between last price and previous day VWAP
    double priceWeeklyVwapDiff = 0.0;          // Difference between last price and weekly VWAP
    double priceBBandUpperDiff = 0.0;        // Difference between last price and upper Bollinger Band
    double priceBBandLowerDiff = 0.0;        // Difference between last price and lower Bollinger Band
    double PriceBBandMiddleDiff = 0.0;       // Difference between last price and middle Bollinger Band

// price-TPO difference value, will be calculated only if price changed
    bool isPriceInVA = false; // true if the last price is within the Value Area, false otherwise
    double priceCurrDayVAHDiff = 0.0;    // Difference between last price and Value Area High of the current underdeveloping profile
    double priceCurrDayVALDiff = 0.0;     // Difference between last price and Value Area Low of the current underdeveloping profile
    double pricePrevDayPOCDiff = 0.0;          // Difference between last price and Point of Control of the previous day profile
    double pricePrevDayVAHDiff = 0.0;   // Difference between last price and previous day Value Area High
    double pricePrevDayVALDiff = 0.0;    // Difference between last price and previous day Value Area

// price S/R difference value, will be calculated only if price changed
    double priceIBHighDiff = 0.0;       // Difference between last price and Initial Balance High
    double priceIBLowDiff = 0.0;        // Difference between last price and Initial Balance Low
    double pricePrevDayHighDiff = 0.0;   // Difference between last price and previous day high
    double pricePrevDayLowDiff = 0.0;    // Difference between last price and previous day low
    double pricePrevDayCloseDiff = 0.0;  // Difference between last price and previous day close
    double priceWeekHighDiff = 0.0;      // Difference between last price and week high
    double priceWeekLowDiff = 0.0;       // Difference between last price and week low

    double priceLastSwingHighDiff = 0.0; // Difference between last price and last swing high
    double priceLastSwingLowDiff = 0.0;  // Difference between last price and last swing low
    double priceLastHVNDiff = 0.0;      // Difference between last price and last high volume node


};

struct Day {
    std::vector<Bar> bars;
    std::string dayOfTheWeek = "-1"; 

// footprint related calculation
    double deltaZscore20bars = 0.0; // Z-score of delta over the last 20 bars
    double cumDelta5barSlope = 0.0; // Slope of cumulative delta over the last 5 bars
    double priceCumDeltaDivergence5bar = 0.0; // Price and cumulative delta divergence measure
    double priceCumDeltaDivergence10bar = 0.0; // Price and cumulative delta divergence measure
    double interactionReversal20bar = 0.0; // Interaction reversal measure over the last 20 bars

// calculated on new bar creation
    double vwap = 0.0; // VWAP for the day
    double vwapUpperStdDev = 0.0; // Standard deviation of VWAP for the day
    double vwapLowerStdDev = 0.0; // Standard deviation of VWAP for the day
    double bbMiddle = 0.0; // Middle Bollinger Band for the day
    double bbUpper = 0.0; // Upper Bollinger Band for the day
    double bbLower = 0.0; // Lower Bollinger Band for the day
    double BBandWidth = 0.0;                     // Width of the Bollinger Bands
    double rsi = 0.0; // Relative Strength Index for the day
    double poc = 0.0; // Point of Control for the day
    double vah = 0.0; // Value Area High for the day
    double val = 0.0; // Value Area Low for the day

//calculated once 
    double ibHigh = 0.0; // Initial Balance High for the day , calculated for the first hour
    double ibLow = 0.0; // Initial Balance Low for the day, calculated for the first hour

    double dayHigh = 0.0; // Highest price of the day calculated at the end of the day
    double dayLow = 0.0; // Lowest price of the day calculated at the end of the day
    double dayClose = 0.0; // Closing price of the day calculated at the end of the day

    double totalVolume = 0.0; // Total volume traded during the day
    double cumulativeDelta = 0.0; // Cumulative delta for the day
    double lastSwingHigh = 0.0; // Last swing high price of the day calculated at bar update
    double lastSwingLow = 0.0; // Last swing low price of the day calculated at bar update
    double lastHighVolumeNode = 0.0; // Last high volume node price of the day calculated at bar update


};

struct Week {
// calculated at the end of the week
    std::vector<Day> days;
    std::string weekOfTheContract = "-1"; // Start date of the week
    double vwap = 0.0; // VWAP for the week
    double poc = 0.0; // Point of Control for the week
    double vah = 0.0; // Value Area High for the week
    double val = 0.0; // Value Area Low for the week
    double weekHigh = 0.0; // Highest price of the week
    double weekLow = 0.0; // Lowest price of the week
};


struct Contract{
    std::vector<Week> weeks;
    std::string contractName = "contractName"; // Name of the futures contract
    
};



#endif // FOOTPRINT_DATA_STRUCTURE_H