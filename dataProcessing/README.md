### CMakeLists.txt

This file contains the CMake configuration for the data processing module. It defines the project, finds necessary packages like SQLite3, and specifies the executable and its source files.

```cmake
cmake_minimum_required(VERSION 3.10)
project(FootprintTraining)

# Find SQLite3
find_package(SQLite3 REQUIRED)

add_executable(footprint_trainer 
    main.cpp 
    database/sqlite.cpp 
    database/json_writer.cpp
    finalProcessing.cpp
    initializeContract.cpp
    src/updateFootprint.cpp
    src/signalCheck.cpp
    src/updateTickSensitiveFeatures.cpp
    src/updatePriceSensitiveFeatures.cpp
    src/finalizeLastBar.cpp
    src/finalizeProcessingDay.cpp
    src/updateBarSensitiveFeatures.cpp
    src/updateDaySensitiveFeatures.cpp
    src/updateWeekSensitiveFeatures.cpp
    src/finalizeWeek.cpp
    src/finalizeContract.cpp
    convertDatesToWeek.cpp
)

target_include_directories(footprint_trainer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(footprint_trainer PRIVATE SQLite::SQLite3)
```

### convertDatesToWeek.cpp

This file contains the implementation for converting a date range into a vector of weeks. It includes helper functions for date manipulation, such as getting the day of the week, adding a day, and subtracting a day.

```cpp
#include <vector>
#include <ctime>
#include <iostream>
#include <array>
#include <iomanip>
#include <chrono>
#include "dataStructure.h"

// Forward declarations
int getDayOfWeek(Date date);
Date addDay(Date date);
Date subtractDay(Date date);
bool operator<=(const Date &a, const Date &b);
bool operator>=(const Date &a, const Date &b);
bool operator<(const Date &a, const Date &b);

// Convert function updated to use Date
std::vector<weekVector> convertDatesToWeeks(Date startDate, Date endDate) {
    std::vector<weekVector> weeks;
    if (endDate < startDate) {
        return weeks; // Return empty vector if date range is invalid
    }

    Date currentDate = startDate;
    int weekCount = 1;

    while (currentDate <= endDate) {
        weekVector currentWeek;
        currentWeek.weekNumber = weekCount++;
        currentWeek.startDate = currentDate;

        while (currentDate <= endDate) {
            int dayOfWeek = getDayOfWeek(currentDate);
            currentWeek.days.push_back({dayOfWeek, currentDate});
            currentWeek.endDate = currentDate;

            currentDate = addDay(currentDate);

            // If the next day is a Monday, this week is over.
            if (getDayOfWeek(currentDate) == 1) {
                break;
            }
        }
        weeks.push_back(currentWeek);
    }
    return weeks;
}

// Helper utilities

static std::tm to_tm(const Date &dt) {
	std::tm tm = {};
	tm.tm_year = dt.y - 1900;
	tm.tm_mon  = dt.m - 1;
	tm.tm_mday = dt.d;
	// use midday to reduce DST issues
	tm.tm_hour = 12;
	tm.tm_min = 0;
	tm.tm_sec = 0;
	return tm;
}

static Date from_tm(const std::tm &tm) {
	return Date{tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday};
}

int getDayOfWeek(Date date) { // 1 = Monday, ..., 7 = Sunday
	std::tm tm = to_tm(date);
	std::time_t t = std::mktime(&tm); // normalizes tm
	std::tm *res = std::localtime(&t);
	int w = res->tm_wday; // 0 = Sunday, 1 = Monday, ...
	return (w == 0) ? 7 : w;
}

Date addDay(Date date) {
	std::tm tm = to_tm(date);
	std::time_t t = std::mktime(&tm);
	t += 24 * 60 * 60;
	std::tm *res = std::localtime(&t);
	return from_tm(*res);
}

Date subtractDay(Date date) {
	std::tm tm = to_tm(date);
	std::time_t t = std::mktime(&tm);
	t -= 24 * 60 * 60;
	std::tm *res = std::localtime(&t);
	return from_tm(*res);
}

// comparison operators
bool operator<=(const Date &a, const Date &b) {
	if (a.y != b.y) return a.y < b.y;
	if (a.m != b.m) return a.m < b.m;
	return a.d <= b.d;
}
bool operator>=(const Date &a, const Date &b) {
	return !(a < b);
}
bool operator<(const Date &a, const Date &b) {
	if (a.y != b.y) return a.y < b.y;
	if (a.m != b.m) return a.m < b.m;
	return a.d < b.d;
}

```

### convertDatesToWeek.h

This is the header file for `convertDatesToWeek.cpp`. It declares the `convertDatesToWeeks` function.

```cpp
#ifndef CONVERTDATESTOWEEK_H
#define CONVERTDATESTOWEEK_H

#include <vector>
#include "dataStructure.h"

// Function declaration
std::vector<weekVector> convertDatesToWeeks(Date startDate, Date endDate);

#endif // CONVERTDATESTOWEEK_H
```

### dataStructure.h

This file defines the core data structures used throughout the application. It includes structures for `Date`, `DayEntry`, `weekVector`, `PriceLevel`, `Footprint`, `Bar`, `Day`, `Week`, and `Contract`.

```cpp
#ifndef FOOTPRINT_DATA_STRUCTURE_H
#define FOOTPRINT_DATA_STRUCTURE_H

#include <map>
#include <vector>
#include <string>
#include <ctime>
#include <array>
#include <cmath>



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
    int64_t bidVolume = 0;
    int64_t askVolume = 0;
    bool isBuyImbalance = false; // true for buy imbalance
    bool isSellImbalance = false; // true for sell imbalance
    int64_t volumeAtPrice = 0;
    int64_t deltaAtPrice = 0;

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
    int buyImbalanceCount = 0; // Number of imbalances detected in the bar
    int sellImbalanceCount = 0; // Number of imbalances detected in the bar
    int delta = 0; // Net volume delta (buy volume - sell volume)
    int barDeltaChange = 0; // Change in delta from the previous bar
    int barHighDelta = 0; // delta of the top two price levels in the bar
    int barLowDelta = 0; // delta of the bottom two price levels in the bar
    double barPOCPrice = 0.0; // Point of Control price level in the bar
    int barPOCVol = 0; // Volume at the Point of Control price level in the bar

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


// extras for calculations
    // rsi
    double prevAvgGain = std::nan("");
    double prevAvgLoss = std::nan("");

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
```
### finalProcessing.cpp

This file contains the main data processing logic. It iterates through weeks and days, fetches tick data from the database, and processes each tick to build the `Contract` data structure.

```cpp
#include "dataStructure.h"
#include <vector>
#include <numeric>
#include <random>
#include <chrono>
#include <ctime>
#include <iostream>
#include "dataStructure.h"
#include "database/database.h"
#include "src/updatefeatures.h"


// this function will take inputs contract and the weekVector and database path
void finalProcessing(double bar_range, double imbalanceThreshhold, Contract& contract , std::vector<weekVector>& weeksVector, const std::string& database_path, const std::string& table_name) {
    // start itteration on the weeksVector
    for (const auto& processing_week : weeksVector) {
        // the start new loop for days in that processing_week
        for (const auto& processing_day : processing_week.days) {
            // fetch the data for that iterated processing_day
            // this is due to the fact that our database has different datetime format
            Date processing_date = {
                processing_day.date.y,
                processing_day.date.m,
                processing_day.date.d
            };
            std::vector<TickData> processing_day_data = fetchData(database_path, table_name, processing_date);
            std::cout << "processing data for :" << processing_date.y << "-" << processing_date.m << "-" << processing_date.d <<"\n";               

                // start new main data processing loop iterating through each row of the fetch data
            for (const auto& row : processing_day_data) {
                if (processing_day_data.empty()) {
                    std::cout << "No data found for date: " << processing_date.y << "-" << processing_date.m << "-" << processing_date.d << std::endl;
                    continue; // Skip to the next day if no data is found
                }
                double currentPrice = row.Price;
                int currentAskVolume = row.AskVolume;
                int currentBidVolume = row.BidVolume;
                std::string currentTime = row.DateTime;

                double lastHigh = contract.weeks.back().days.back().bars.back().high;
                double lastLow = contract.weeks.back().days.back().bars.back().low;
            
                // check if the price is in the range of the current bar, this due to the fact that we are processing chart in ranged bar
                if (lastHigh - currentPrice <= bar_range && currentPrice - lastLow <= bar_range )  {          //if price is in the range so only updation of the bar
                    if (currentPrice != contract.weeks.back().days.back().bars.back().close) {           //if price changed
                        // updating bar's ohlc
                        contract.weeks.back().days.back().bars.back().close = currentPrice;
                        contract.weeks.back().days.back().bars.back().high = std::max(lastHigh, currentPrice);
                        contract.weeks.back().days.back().bars.back().low = std::min(lastLow, currentPrice);
                        contract.weeks.back().days.back().bars.back().endTime = currentTime;

                        // update footprint bar
                        auto imbalance_change = updateFootprint(contract, imbalanceThreshhold, currentPrice, currentAskVolume, currentBidVolume);

                        checkForSignal(contract);      //check for signal
                        // if signal 
                            //update all
                            // append to signal data structure

                        // update tick change sensitive features
                        updateTickSensitiveFeatures(contract, currentPrice, currentAskVolume, currentBidVolume, imbalance_change);

                        // update price change sensitive feartures
                        updatePriceSensitiveFeatures(contract, currentPrice, currentAskVolume, currentBidVolume);
                    }    

                    else  {         //else price not changed
                        // update footprint bar
                        auto imbalance_change = updateFootprint(contract, imbalanceThreshhold, currentPrice, currentAskVolume, currentBidVolume);

                        checkForSignal(contract);      //check for signal
                            // if signal
                                //update all
                                // append to signal data structure

                        // update tick change sensitive features
                        updateTickSensitiveFeatures(contract, currentPrice, currentAskVolume, currentBidVolume, imbalance_change);

                    }
                } 
                else {           //else price not in the range, so we need to create new bar
                    // finalize the last bar and update bar change sensitive features
                    // and add new bar in the bars vector of the currect processing day's data structure
                    updateBarChangeSensitiveFeatures(contract, currentPrice, currentAskVolume, currentBidVolume);
                    finalizeLastBar(contract, currentTime, currentPrice, currentAskVolume, currentBidVolume);
                    
                    // update footprint bar
                    auto imbalance_change = updateFootprint(contract, imbalanceThreshhold, currentPrice, currentAskVolume, currentBidVolume);

                    // update tick change sensitive features
                    updateTickSensitiveFeatures(contract, currentPrice, currentAskVolume, currentBidVolume, imbalance_change);

                    // update price change sensitive feartures
                    updatePriceSensitiveFeatures(contract, currentPrice, currentAskVolume, currentBidVolume);

                }
            }
            // finalize the processing_day and update day change sensitive features
        updateDayChangeSensitiveFeatures(contract);
        finalizeProcessingDay(contract);
        }
        // finalize the processing_week and update week change sensitive features
        updateWeekChangeSensitiveFeatures(contract);
        finalizeProcessingWeek(contract);

    }
    // finalize the contract
    finalizeContract(contract);
}
```

### initializeContract.cpp

This file contains the logic for initializing a new `Contract` data structure. It fetches the first tick of a given start date to set the initial price for the contract, week, day, and bar.

```cpp
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
    initialBar.prevAvgGain = std::nan("");
    initialBar.prevAvgLoss = std::nan("");

    // Initialize price-related fields in Day
    Day initialDay;
    initialDay.vwap = initialPrice;
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
```

### main.cpp

This is the main entry point of the application. It parses command-line arguments, initializes the `Contract` data structure, converts the date range to weeks, calls the main processing function, and writes the final `Contract` data to a JSON file.

```cpp
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "dataStructure.h"
#include "database/json_writer.h"
#include "convertDatesToWeek.h"
#include "database/database.h"
#include "database/json_writer.h"

#include <chrono>

extern void initializeContract(Contract& contract, const std::string& database_path, const std::string& table_name, const Date& startDate);
extern void finalProcessing(double bar_range, double imbalanceThreshhold, Contract& contract , std::vector<weekVector>& weeksVector, const std::string& database_path, const std::string& table_name);

//test usage: ./footprint_trainer <bar_range> <database_path> <table_name> <start_date> <end_date> <output_directory> <imbalanceThreshhold>
//test usage: ./footprint_trainer 2.5 "/home/sarjil/sarjil_u/C++/footprintTradingBot/testData/converted_database.db" ESH24_tick 2024-02-13 2024-03-13 "/media/sarjil/Vol2/Data/dataFromFootprint/" 3.0

int main(int argc, char* argv[]) {
    if (argc != 8) {
        std::cerr << "Usage: " << argv[0] << " <bar_range> <database_path> <table_name> <start_date> <end_date> <output_directory> <imbalanceThreshhold>" << std::endl;

        return 1;
    }
    const double bar_range = std::stod(argv[1]);
    const std::string database_path(argv[2]);
    const std::string table_name(argv[3]);
    const std::string start_date(argv[4]);
    const std::string end_date(argv[5]);
    const std::string output_dir(argv[6]);
    const double imbalanceThreshhold = std::stod(argv[7]);

    std::cout << "Bar range: " << bar_range << std::endl;
    std::cout << "Database path: " << database_path << std::endl;
    std::cout << "Table name: " << table_name << std::endl;
    std::cout << "Start date: " << start_date << std::endl;
    std::cout << "End date: " << end_date << std::endl;
    std::cout << "Output directory: " << output_dir << std::endl;
    
    // Parse dates in format YYYY-MM-DD
    // Date struct defined in convertDatesToWeek.cpp
    std::istringstream startStream(start_date);
    std::istringstream endStream(end_date);
    Date startDate, endDate;
    char delimiter;
    if (!(startStream >> startDate.y >> delimiter >> startDate.m >> delimiter >> startDate.d)) {
        throw std::runtime_error("Invalid date format. Expected YYYY-MM-DD");
    }
    endStream >> endDate.y >> delimiter >> endDate.m >> delimiter >> endDate.d;
    

//-----------------------------------------------------------------------------------------------------------------
    
    // initialize the contract and signal datastructure
    Contract contract;
    initializeContract(contract, database_path, table_name, startDate);
    contract.contractName = table_name;
    std::cout << "Initialized contract for: " << contract.contractName << std::endl;

    // Signal signalData;

    // convert date range to weeksVector
    auto weeksVector = convertDatesToWeeks(startDate, endDate);
    // printing the weeksVector for verification
    for (const auto& week : weeksVector) {
        // Ensure the week is not empty before accessing its days
        if (!week.days.empty()) {
            // The start date is the date of the first day in the week
            const auto& weekStartDate = week.days.front().date;
            // The end date is the date of the last day in the week
            const auto& weekEndDate = week.days.back().date;

            std::cout << "Week Start: " << weekStartDate.y << "-" << weekStartDate.m << "-" << weekStartDate.d
                      << ", Week End: " << weekEndDate.y << "-" << weekEndDate.m << "-" << weekEndDate.d << std::endl;
        }
    }


    auto start_time = std::chrono::high_resolution_clock::now();
//-----------------------------------------------------------------------------------------------------------------

    // call the finalProcessing function which accepts the contract by reference 
    finalProcessing(bar_range,imbalanceThreshhold, contract, weeksVector, database_path,  table_name);
//     and the weeksVector by reference and database path and table name and also signal structure by reference
    
    
    
    // then final processed contract structure will be saved as json file using the writeContractToJson function from json_writer.h and save it to provided output directory path
    std::cout << "Writing contract data to JSON in directory: " << output_dir << std::endl;
    writeContractToJson(contract, output_dir);


    // then save the signal structure in the signal database and also in the csv format to the provided output directory path
    

//------------------------------------------------------------------------------------------
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_seconds = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
    auto duration_minutes = std::chrono::duration_cast<std::chrono::minutes>(end_time - start_time);
    std::cout << "\n-------------------------------------------------" << std::endl;
    std::cout << "Processing completed successfully." << std::endl;
    std::cout << "Total execution time: " << duration_seconds.count() << " seconds (" 
              << duration_minutes.count() << " minutes)." << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;

    return 0;
}
```

### database/database.h

This header file declares the functions for interacting with the SQLite database. It defines the `TickData` structure and declares the `fetchData` and `fetchFirstTick` functions.

```cpp
#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <sqlite3.h>
#include "../dataStructure.h"

struct TickData {
    int id;
    std::string DateTime;
    double Price;
    double AskVolume;
    double BidVolume;
};


std::vector<TickData> fetchData(const std::string& database_path, const std::string& table_name, const Date date);

TickData fetchFirstTick(const std::string& database_path, const std::string& table_name, const Date date);


#endif // DATABASE_H
```

### database/json_writer.cpp

This file contains the implementation for writing the `Contract` data structure to a JSON file. It includes helper functions for serializing each part of the `Contract` structure into JSON format.

```cpp
#include "json_writer.h"
#include <iostream>
#include <fstream>
#include <sstream>

// Forward declarations for serialization functions
std::string priceLevelToJson(double price, const PriceLevel& pl);
std::string footprintToJson(const Footprint& fp);
std::string barToJson(const Bar& bar);
std::string dayToJson(const Day& day);
std::string weekToJson(const Week& week);
std::string contractToJson(const Contract& contract);

std::string priceLevelToJson(double price, const PriceLevel& pl) {
    std::stringstream ss;
    ss << "{";
    ss << \"price\": " << price << ",";
    ss << \"bidVolume\": " << pl.bidVolume << ",";
    ss << \"askVolume\": " << pl.askVolume << ",";
    ss << \"isBuyImbalance\": " << (pl.isBuyImbalance ? "true" : "false") << ",";
    ss << \"isSellImbalance\": " << (pl.isSellImbalance ? "true" : "false") << ",";
    ss << \"volumeAtPrice\": " << pl.volumeAtPrice << ",";
    ss << \"deltaAtPrice\": " << pl.deltaAtPrice;
    ss << "}";
    return ss.str();
}

std::string footprintToJson(const Footprint& fp) {
    std::stringstream ss;
    ss << "[";
    bool first = true;
    for (const auto& [price, level] : fp.priceLevels) {
        if (!first) {
            ss << ",";
        }
        ss << priceLevelToJson(price, level);
        first = false;
    }
    ss << "]";
    return ss.str();
}

std::string barToJson(const Bar& bar) {
    std::stringstream ss;
    ss << "{";
    ss << \"startTime\": \"" << bar.startTime << "\",";
    ss << \"endTime\": \"" << bar.endTime << "\",";
    ss << \"open\": " << bar.open << ",";
    ss << \"high\": " << bar.high << ",";
    ss << \"low\": " << bar.low << ",";
    ss << \"close\": " << bar.close << ",";
    ss << \"barTotalVolume\": " << bar.barTotalVolume << ",";
    ss << \"footprint\": " << footprintToJson(bar.footprint) << ",";
    ss << \"buyImbalanceCount\": " << bar.buyImbalanceCount << ",";
    ss << \"sellImbalanceCount\": " << bar.sellImbalanceCount << ",";
    ss << \"delta\": " << bar.delta << ",";
    ss << \"barDeltaChange\": " << bar.barDeltaChange << ",";
    ss << \"barHighDelta\": " << bar.barHighDelta << ",";
    ss << \"barLowDelta\": " << bar.barLowDelta << ",";
    ss << \"barPOCPrice\": " << bar.barPOCPrice << ",";
    ss << \"barPOCVol\": " << bar.barPOCVol << ",";    
    ss << \"signal\": " << bar.signal << ",";
    ss << \"signalID\": " << bar.signalID << ",";
    ss << \"signalStatus\": " << (bar.signalStatus ? "true" : "false") << ",";
    ss << \"priceCurrentDayVwapDiff\": " << bar.priceCurrentDayVwapDiff << ",";
    ss << \"priceCurrentDayVwapUpperStdDevDiff\": " << bar.priceCurrentDayVwapUpperStdDevDiff << ",";
    ss << \"priceCurrentDayVwapLowerStdDevDiff\": " << bar.priceCurrentDayVwapLowerStdDevDiff << ",";
    ss << \"pricePreviousDayVwapDiff\": " << bar.pricePreviousDayVwapDiff << ",";
    ss << \"priceWeeklyVwapDiff\": " << bar.priceWeeklyVwapDiff << ",";
    ss << \"priceBBandUpperDiff\": " << bar.priceBBandUpperDiff << ",";
    ss << \"priceBBandLowerDiff\": " << bar.priceBBandLowerDiff << ",";
    ss << \"PriceBBandMiddleDiff\": " << bar.PriceBBandMiddleDiff << ",";
    ss << \"isPriceInVA\": " << (bar.isPriceInVA ? "true" : "false") << ",";
    ss << \"priceCurrDayVAHDiff\": " << bar.priceCurrDayVAHDiff << ",";
    ss << \"priceCurrDayVALDiff\": " << bar.priceCurrDayVALDiff << ",";
    ss << \"pricePrevDayPOCDiff\": " << bar.pricePrevDayPOCDiff << ",";
    ss << \"pricePrevDayVAHDiff\": " << bar.pricePrevDayVAHDiff << ",";
    ss << \"pricePrevDayVALDiff\": " << bar.pricePrevDayVALDiff << ",";
    ss << \"priceIBHighDiff\": " << bar.priceIBHighDiff << ",";
    ss << \"priceIBLowDiff\": " << bar.priceIBLowDiff << ",";
    ss << \"pricePrevDayHighDiff\": " << bar.pricePrevDayHighDiff << ",";
    ss << \"pricePrevDayLowDiff\": " << bar.pricePrevDayLowDiff << ",";
    ss << \"pricePrevDayCloseDiff\": " << bar.pricePrevDayCloseDiff << ",";
    ss << \"priceWeekHighDiff\": " << bar.priceWeekHighDiff << ",";
    ss << \"priceWeekLowDiff\": " << bar.priceWeekLowDiff << ",";
    ss << \"priceLastSwingHighDiff\": " << bar.priceLastSwingHighDiff << ",";
    ss << \"priceLastSwingLowDiff\": " << bar.priceLastSwingLowDiff << ",";
    ss << \"priceLastHVNDiff\": " << bar.priceLastHVNDiff;
    ss << "}";
    return ss.str();
}

std::string dayToJson(const Day& day) {
    std::stringstream ss;
    ss << "{";
    ss << \"dayOfTheWeek\": \"" << day.dayOfTheWeek << "\",";
    ss << \"bars\": [";
    bool first = true;
    for (const auto& bar : day.bars) {
        if (!first) {
            ss << ",";
        }
        ss << barToJson(bar);
        first = false;
    }
    ss << "]";
    ss << ",";
    ss << \"deltaZscore20bars\": " << day.deltaZscore20bars << ",";
    ss << \"cumDelta5barSlope\": " << day.cumDelta5barSlope << ",";
    ss << \"priceCumDeltaDivergence5bar\": " << day.priceCumDeltaDivergence5bar << ",";
    ss << \"priceCumDeltaDivergence10bar\": " << day.priceCumDeltaDivergence10bar << ",";
    ss << \"interactionReversal20bar\": " << day.interactionReversal20bar << ",";
    ss << \"vwap\": " << day.vwap << ",";
    ss << \"vwapUpperStdDev\": " << day.vwapUpperStdDev << ",";
    ss << \"vwapLowerStdDev\": " << day.vwapLowerStdDev << ",";
    ss << \"bbMiddle\": " << day.bbMiddle << ",";
    ss << \"bbUpper\": " << day.bbUpper << ",";
    ss << \"bbLower\": " << day.bbLower << ",";
    ss << \"BBandWidth\": " << day.BBandWidth << ",";
    ss << \"rsi\": " << day.rsi << ",";
    ss << \"poc\": " << day.poc << ",";
    ss << \"vah\": " << day.vah << ",";
    ss << \"val\": " << day.val << ",";
    ss << \"ibHigh\": " << day.ibHigh << ",";
    ss << \"ibLow\": " << day.ibLow << ",";
    ss << \"dayHigh\": " << day.dayHigh << ",";
    ss << \"dayLow\": " << day.dayLow << ",";
    ss << \"dayClose\": " << day.dayClose << ",";
    ss << \"totalVolume\": " << day.totalVolume << ",";
    ss << \"cumulativeDelta\": " << day.cumulativeDelta << ",";
    ss << \"lastSwingHigh\": " << day.lastSwingHigh << ",";
    ss << \"lastSwingLow\": " << day.lastSwingLow << ",";
    ss << \"lastHighVolumeNode\": " << day.lastHighVolumeNode;
    ss << "}";
    return ss.str();
}

std::string weekToJson(const Week& week) {
    std::stringstream ss;
    ss << "{";
    ss << \"weekOfTheContract\": \"" << week.weekOfTheContract << "\",";
    ss << \"days\": [";
    bool first = true;
    for (const auto& day : week.days) {
        if (!first) {
            ss << ",";
        }
        ss << dayToJson(day);
        first = false;
    }
    ss << "]";
    ss << ",";
    ss << \"vwap\": " << week.vwap << ",";
    ss << \"poc\": " << week.poc << ",";
    ss << \"vah\": " << week.vah << ",";
    ss << \"val\": " << week.val << ",";
    ss << \"weekHigh\": " << week.weekHigh << ",";
    ss << \"weekLow\": " << week.weekLow;
    ss << "}";
    return ss.str();
}

std::string contractToJson(const Contract& contract) {
    std::stringstream ss;
    ss << "{";
    ss << \"contractName\": \"" << contract.contractName << "\",";
    ss << \"weeks\": [";
    bool first = true;
    for (const auto& week : contract.weeks) {
        if (!first) {
            ss << ",";
        }
        ss << weekToJson(week);
        first = false;
    }
    ss << "]";
    ss << "}";
    return ss.str();
}

void writeContractToJson(const Contract& contract, const std::string& output_dir) {
    std::string json_string = contractToJson(contract);
    std::ofstream outfile(output_dir + "/contract.json");
    outfile << json_string;
    outfile.close();
}
```

### database/json_writer.h

This is the header file for `json_writer.cpp`. It declares the `writeContractToJson` function.

```cpp
#ifndef JSON_WRITER_H
#define JSON_WRITER_H

#include "../dataStructure.h"
#include <string>

void writeContractToJson(const Contract& contract, const std::string& output_dir);

#endif // JSON_WRITER_H
```

### database/sqlite.cpp

This file contains the implementation for interacting with the SQLite database. It includes the `fetchData` function to retrieve tick data for a specific date and the `fetchFirstTick` function to get the first tick of a given date.

```cpp
#include <string>
#include <vector>
#include <sqlite3.h>
#include <iostream>
#include "database.h"
#include "../dataStructure.h"


std::vector<TickData> fetchData(const std::string& database_path, const std::string& table_name, const Date date) {
    std::vector<TickData> result;
    sqlite3* db;
    
    // Open database
    int rc = sqlite3_open(database_path.c_str(), &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return result;
    }

    // Format date string YYYY-MM-DD
    char date_str[11];
    snprintf(date_str, sizeof(date_str), "%04d-%02d-%02d", date.y, date.m, date.d);

    // Prepare SQL statement with proper date filtering
    std::string sql = "SELECT id, DateTime, Price, AskVolume, BidVolume FROM " + 
                     table_name + 
                     " WHERE strftime('%Y-%m-%d', DateTime) = ?";
    
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to fetch data: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return result;
    }

    // Bind date parameter
    sqlite3_bind_text(stmt, 1, date_str, -1, SQLITE_STATIC);

    // Execute query and fetch results
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TickData tick;
        tick.id = sqlite3_column_int(stmt, 0);
        tick.DateTime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        tick.Price = sqlite3_column_double(stmt, 2);
        tick.AskVolume = sqlite3_column_double(stmt, 3);
        tick.BidVolume = sqlite3_column_double(stmt, 4);
        result.push_back(tick);
    }

    // Clean up
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}



//this function is to initialize the contract
TickData fetchFirstTick(const std::string& database_path, const std::string& table_name, const Date date) {
    TickData result = {}; // Initialize to default values
    result.id = -1; // Use -1 to indicate failure or no data found
    sqlite3* db;
    
    // Open database
    int rc = sqlite3_open(database_path.c_str(), &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return result;
    }

    // Format date string YYYY-MM-DD
    char date_str[11];
    snprintf(date_str, sizeof(date_str), "%04d-%02d-%02d", date.y, date.m, date.d);

    // Prepare SQL statement to get the first row of the day
    std::string sql = "SELECT id, DateTime, Price, AskVolume, BidVolume FROM " + 
                     table_name + 
                     " WHERE strftime('%Y-%m-%d', DateTime) = ? ORDER BY DateTime ASC LIMIT 1";
    
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to fetch data: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return result;
    }

    // Bind date parameter
    sqlite3_bind_text(stmt, 1, date_str, -1, SQLITE_STATIC);

    // Execute query and fetch the first result
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result.id = sqlite3_column_int(stmt, 0);
        result.DateTime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        result.Price = sqlite3_column_double(stmt, 2);
        result.AskVolume = sqlite3_column_double(stmt, 3);
        result.BidVolume = sqlite3_column_double(stmt, 4);
    }

    // Clean up
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}
```

### python/json_to_parquet.py

This Python script converts the JSON output of the C++ data processing into a Parquet file. It flattens the nested JSON structure into a list of bars, converts it to a pandas DataFrame, and then saves it as a Parquet file for efficient storage and retrieval.

```python
import json
import pandas as pd
import pyarrow as pa
import pyarrow.parquet as pq
import argparse

def json_to_parquet(json_file_path, parquet_file_path):
    """Reads a JSON file containing contract data, flattens it into a list of bars,
    converts it to a pandas DataFrame, and saves it as a Parquet file.
    """

    print(f"Reading JSON file: {json_file_path}")
    with open(json_file_path, 'r') as f:
        contract_data = json.load(f)

    print("Flattening JSON data...")
    bars_list = []
    for week in contract_data['weeks']:
        for day in week['days']:
            for bar in day['bars']:
                # You can add more fields from the day or week level here if needed
                bar['dayOfTheWeek'] = day['dayOfTheWeek']
                bar['weekOfTheContract'] = week['weekOfTheContract']
                bars_list.append(bar)

    print("Creating pandas DataFrame...")
    df = pd.json_normalize(bars_list, sep='_')

    print("Converting DataFrame to Arrow Table...")
    table = pa.Table.from_pandas(df)

    print(f"Writing Parquet file: {parquet_file_path}")
    pq.write_table(table, parquet_file_path)

    print("Conversion complete.")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Convert contract JSON to Parquet.')
    parser.add_argument('json_file', help='Input JSON file path.')
    parser.add_argument('parquet_file', help='Output Parquet file path.')
    args = parser.parse_args()

    json_to_parquet(args.json_file, args.parquet_file)
```

### python/readParquet.py

This Python script demonstrates how to read the generated Parquet file using pandas and print the data types of the columns.

```python
import pandas as pd

df = pd.read_parquet('contract.parquet')

print("\nColumn data types:")
print(df.dtypes)
```

### src/finalizeContract.cpp

This file contains the `finalizeContract` function, which is called at the end of the data processing to perform any finalization steps on the `Contract` data structure.

```cpp
#include "../dataStructure.h"



void finalizeContract(Contract& contract) {
    // Finalize the contract and update any remaining features
    // Currently, no specific finalization logic is implemented
    // This function can be expanded as needed in the future
    


}
```

### src/finalizeLastBar.cpp

This file contains the `finalizeLastBar` function, responsible for finalizing the current bar and initializing a new one when a new bar is created (e.g., when the price moves out of the current bar's range).

```cpp
#include "../dataStructure.h"

// this function will do the following
    // first of all add new bar in the bars vector of the current day
        // initialize all the features of the new bar

void finalizeLastBar(Contract& contract, std::string currentTime, double currentPrice, int currentAskVolume, int currentBidVolume) {
    auto& DAY = contract.weeks.back().days.back();

    // UPDATE THE END TIME OF THE LAST BAR
    DAY.bars.back().endTime = currentTime;

    // first add new bar at the end of the bars vector
    //but first initializing values
    Bar newBar;
    newBar.startTime = currentTime;
    newBar.open = currentPrice;
    newBar.high = currentPrice;
    newBar.low = currentPrice;
    newBar.close = currentPrice;
    newBar.barTotalVolume = currentAskVolume + currentBidVolume;
    newBar.startTime = currentTime;
    newBar.endTime = "-1";
    newBar.barDeltaChange = 0;
    newBar.barHighDelta = 0;
    newBar.barLowDelta = 0;
    newBar.signal = 0;
    newBar.signalID = -1;
    newBar.signalStatus = false;
    newBar.priceBBandLowerDiff = 0.0;
    newBar.priceBBandUpperDiff = 0.0;

    PriceLevel newPriceLevel;
    Footprint& newFootprint = newBar.footprint;
    newPriceLevel.bidVolume = currentBidVolume;
    newPriceLevel.askVolume = currentAskVolume;
    newPriceLevel.isBuyImbalance = false;
    newPriceLevel.isSellImbalance = false;
    newPriceLevel.volumeAtPrice = (currentBidVolume + currentAskVolume);
    newPriceLevel.deltaAtPrice = (currentAskVolume - currentBidVolume);
    newFootprint.priceLevels[currentPrice] = newPriceLevel;

    //addding new bar
    
    DAY.bars.push_back(newBar);
    DAY.bars.back().footprint.priceLevels[currentPrice] = newPriceLevel;


}
```

### src/finalizeProcessingDay.cpp

This file contains the `finalizeProcessingDay` function, which is responsible for finalizing the current day's data and preparing for a new day within the `Contract` structure.

```cpp
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
```

### src/finalizeWeek.cpp

This file contains the `finalizeProcessingWeek` function, which is responsible for finalizing the current week's data and preparing for a new week within the `Contract` structure.

```cpp
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
```

### src/signalCheck.cpp

This file contains the `checkForSignal` function, which is responsible for checking for trading signals based on the current bar's data. Currently, it contains a demo logic that sets a signal.

```cpp
#include "../dataStructure.h"



void checkForSignal(Contract& contract){
    auto& BAR = contract.weeks.back().days.back().bars.back();
    // this is the demo logic for signal checking
    BAR.signal = 1;
    BAR.signalID = 1;
    BAR.signalStatus = true;

}
```

### src/updateBarSensitiveFeatures.cpp

This file contains the `updateBarChangeSensitiveFeatures` function, which is responsible for updating features that are sensitive to changes in the bar (e.g., when a new bar is formed).

```cpp
#include "../dataStructure.h"



void updateBarChangeSensitiveFeatures(Contract& contract, double currentPrice, int currentAskVolume, int currentBidVolume) {
    // Update bar change sensitive features
    auto& BAR = contract.weeks.back().days.back().bars.back();

    
    
}
```

### src/updateDaySensitiveFeatures.cpp

This file contains the `updateDayChangeSensitiveFeatures` function, which is responsible for updating features that are sensitive to changes in the day (e.g., at the end of a trading day).

```cpp
#include "../dataStructure.h"



void updateDayChangeSensitiveFeatures(Contract& contract) {
    // Update day change sensitive features
    auto& DAY = contract.weeks.back().days.back();

    DAY.dayHigh = 0.0;
    DAY.dayLow = 0.0;
    DAY.dayClose = 0.0;
    DAY.totalVolume = 0.0;
    DAY.cumulativeDelta = 0.0;
    DAY.lastSwingHigh = 0.0;
    DAY.lastSwingLow = 0.0;
    DAY.lastHighVolumeNode = 0.0;
    DAY.ibHigh = 0.0;
    DAY.ibLow = 0.0;
    DAY.vwap = 0.0;
    DAY.vwapUpperStdDev = 0.0;
    DAY.vwapLowerStdDev = 0.0;
    DAY.bbMiddle = 0.0;
    DAY.bbUpper = 0.0;
    DAY.bbLower = 0.0;
    DAY.BBandWidth = 0.0;
    DAY.rsi = 0.0;
    DAY.poc = 0.0;
    DAY.vah = 0.0;
    DAY.val = 0.0;
    DAY.deltaZscore20bars = 0.0;
    DAY.cumDelta5barSlope = 0.0;
    DAY.priceCumDeltaDivergence5bar = 0.0;
    DAY.priceCumDeltaDivergence10bar = 0.0;
    DAY.interactionReversal20bar = 0.0;
    
    
}
```

### src/updatefeatures.h

This header file declares various functions responsible for updating different features of the `Contract` data structure, including signal checking, footprint updates, and features sensitive to tick, price, bar, day, and week changes.

```cpp
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

void finalizeLastBar(Contract& contract, std::string datetime, double currentPrice, int currentAskVolume, int currentBidVolume);
void finalizeProcessingDay(Contract& contract);
void finalizeProcessingWeek(Contract& contract);
void finalizeContract(Contract& contract);




#endif // UPDATEFEATURES_H
```

### src/updateFootprint.cpp

This file contains the `updateFootprint` function, which updates the footprint data for the current bar based on incoming tick data. It also calculates and tracks buy and sell imbalances.

```cpp
#include "../dataStructure.h"



std::pair<int, int> updateFootprint(Contract& contract, double imbalanceThreshhold, double price, int bidVolume, int askVolume) {
    // if price level exists, update volumes
    auto& FOOTPRINT = contract.weeks.back().days.back().bars.back().footprint;
    if (FOOTPRINT.priceLevels.find(price) != FOOTPRINT.priceLevels.end()){
        FOOTPRINT.priceLevels[price].bidVolume += bidVolume;
        FOOTPRINT.priceLevels[price].askVolume += askVolume;
        FOOTPRINT.priceLevels[price].volumeAtPrice += (bidVolume + askVolume);
        FOOTPRINT.priceLevels[price].deltaAtPrice += (askVolume - bidVolume);
        
    }
    else{
    // else create new price level and set volumes
    PriceLevel newLevel;
    newLevel.bidVolume = bidVolume;
    newLevel.askVolume = askVolume;
    newLevel.isBuyImbalance = false;
    newLevel.isSellImbalance = false;
    newLevel.volumeAtPrice = (bidVolume + askVolume);
    newLevel.deltaAtPrice = (askVolume - bidVolume);
    FOOTPRINT.priceLevels[price] = newLevel;
    }
    


    if (FOOTPRINT.priceLevels.size() < 2) {
        return {0, 0}; // Not enough levels to have an imbalance, no change. 
    }

    // --- 2. Track Imbalance Changes ---
    // A single tick can affect the imbalance status of up to three price levels:
    // - The Buy Imbalance at the level ABOVE the current price.
    // - The Buy and Sell Imbalance at the CURRENT price.
    // - The Sell Imbalance at the level BELOW the current price.
    // We must check all of them and calculate the net change.

    int buy_imb_change = 0;
    int sell_imb_change = 0;

    // --- Helper Lambda to check and update an imbalance flag ---
    // This avoids code duplication and makes the logic clearer.
    auto check_and_update =[](bool& flag, int64_t aggressive_vol, int64_t passive_vol, double ratio) {
        bool old_state = flag;
        bool new_state = false;
        if (passive_vol == 0 && aggressive_vol > 0) {
            new_state = true;
        } else if (passive_vol > 0 && (static_cast<double>(aggressive_vol) / passive_vol) >= ratio) {
            new_state = true;
        }
        flag = new_state;
        return (new_state - old_state); // Returns +1 if created, -1 if removed, 0 if unchanged
    };



    //calculate imbalance
    // --- New Imbalance Calculation Logic ---
    if (FOOTPRINT.priceLevels.size() >= 2) {
        // Use safe iterators to determine if the current price is at the top or bottom
        bool isLowestPrice = (FOOTPRINT.priceLevels.begin()->first == price);
        bool isHighestPrice = (FOOTPRINT.priceLevels.rbegin()->first == price);

        // Case 1: Current price is the LOWEST in the footprint
        if (isLowestPrice) {
            // A) Calculate SELL imbalance at the current price (price)
            sell_imb_change += check_and_update(
            FOOTPRINT.priceLevels[price].isSellImbalance, // imbalance flag
            FOOTPRINT.priceLevels[price].bidVolume, // aggressor
            FOOTPRINT.priceLevels[price + 0.25].askVolume,
            imbalanceThreshhold );

            // B) Calculate BUY imbalance at the next price level (price + 0.25)
            buy_imb_change += check_and_update(
            FOOTPRINT.priceLevels[price+0.25].isBuyImbalance, // imbalance flag
            FOOTPRINT.priceLevels[price+0.25].askVolume, // The value that just changed
            FOOTPRINT.priceLevels[price].bidVolume,
            imbalanceThreshhold );
        }
        // Case 2: Current price is the HIGHEST in the footprint
        else if (isHighestPrice) {
            // A) Calculate BUY imbalance at the current price (price)
            buy_imb_change += check_and_update(
            FOOTPRINT.priceLevels[price].isBuyImbalance, // imbalance flag
            FOOTPRINT.priceLevels[price].askVolume, // aggressor
            FOOTPRINT.priceLevels[price-0.25].bidVolume,
            imbalanceThreshhold );

            // B) Calculate SELL imbalance at the previous price level (price - 0.25)
            sell_imb_change += check_and_update(
            FOOTPRINT.priceLevels[price-0.25].isSellImbalance, // imbalance flag
            FOOTPRINT.priceLevels[price-0.25].bidVolume, // aggressor
            FOOTPRINT.priceLevels[price].askVolume,
            imbalanceThreshhold );
        }
        // Case 3: Current price is in the MIDDLE of the footprint
        else {
            // A) Calculate BUY imbalance at the current price (price)
            buy_imb_change += check_and_update(
            FOOTPRINT.priceLevels[price].isBuyImbalance, // imbalance flag
            FOOTPRINT.priceLevels[price].askVolume, // aggressor
            FOOTPRINT.priceLevels[price-0.25].bidVolume,
            imbalanceThreshhold );

            // B) Calculate SELL imbalance at the current price (price)
            sell_imb_change += check_and_update(
            FOOTPRINT.priceLevels[price].isSellImbalance, // imbalance flag
            FOOTPRINT.priceLevels[price].bidVolume, // aggressor
            FOOTPRINT.priceLevels[price + 0.25].askVolume,
            imbalanceThreshhold );

            // C) Calculate BUY imbalance at the next price level (price + 0.25)
            buy_imb_change += check_and_update(
            FOOTPRINT.priceLevels[price+0.25].isBuyImbalance, // imbalance flag
            FOOTPRINT.priceLevels[price+0.25].askVolume, // The value that just changed
            FOOTPRINT.priceLevels[price].bidVolume,
            imbalanceThreshhold );

            // D) Calculate SELL imbalance at the previous price level (price - 0.25)
            sell_imb_change += check_and_update(
            FOOTPRINT.priceLevels[price-0.25].isSellImbalance, // imbalance flag
            FOOTPRINT.priceLevels[price-0.25].bidVolume, // aggressor
            FOOTPRINT.priceLevels[price].askVolume,
            imbalanceThreshhold );
        }
    }

    // --- 4. Return the net changes ---
    return {buy_imb_change, sell_imb_change};
}
```

### src/updatePriceSensitiveFeatures.cpp

This file contains the `updatePriceSensitiveFeatures` function, which is responsible for updating features that are sensitive to price changes, such as differences from VWAP, Bollinger Bands, TPO levels, and support/resistance levels.

```cpp
#include "../dataStructure.h"




// price-TPO difference
    // price-VAH diff
    // price-VAL diff
    // price-POCDiff (current day poc)
    // price-POCDiff (previous day poc)
    // isPriceInVA
    // 
// price-indicator difference
    // price-VWAP diff
    // price-VWAP std diff
    // price-BBU diff
    // price-BBL diff
    // price-BBM diff

// S/R diff
    // price-prevDayHigh diff
    // price-previousDayLow diff
    // price-IBHigh diff
    // pirce-IBLow diff


void updatePriceSensitiveFeatures(Contract& contract, double currentPrice, int currentAskVolume, int currentBidVolume) {
    // Update footprint
    auto& DAY = contract.weeks.back().days.back();

    
    
}
```

### src/updateTickSensitiveFeatures.cpp

This file contains the `updateTickSensitiveFeatures` function, which updates various features of the current bar that are sensitive to every incoming tick, including total volume, delta, imbalance counts, and POC.

```cpp
#include "../dataStructure.h"



// feature that are sensitive to every tick update
// signal and its derivatives will be checked seperately
  // basic bar features -----------------------------------------------
    // barTotalVolume

  // footprint features -----------------------------------------------
    // buyImbalanceCount
    // sellImbalanceCount
    // barDelta
    // barDeltaHigh
    // barDeltaLow
    // barDeltaChange

    //TPO features ------------------------------------------------
    // barPOC


void updateTickSensitiveFeatures(Contract& contract, double currentPrice, int currentAskVolume, int currentBidVolume, std::pair<int, int> imbalanceChange) {
    // Update footprint
    auto& BAR = contract.weeks.back().days.back().bars.back();

    // basic
    BAR.barTotalVolume += (currentAskVolume + currentBidVolume);

    //footprint matrics
    BAR.delta += (currentAskVolume - currentBidVolume);
    // BAR.barDeltaChange = 
    BAR.buyImbalanceCount += imbalanceChange.first;
    BAR.sellImbalanceCount += imbalanceChange.second;


        // updating poc price and volume 
    const auto& PRICELEVEL = BAR.footprint.priceLevels.at(currentPrice);
    if (PRICELEVEL.volumeAtPrice > BAR.barPOCVol){
        BAR.barPOCPrice = currentPrice;
        BAR.barPOCVol = PRICELEVEL.volumeAtPrice;
    
    }



    // std::map is sorted, so rbegin() and begin() give us fast access to extremes.
    auto& priceLevels = BAR.footprint.priceLevels;
    if (!priceLevels.empty()) {
        // Bar High Delta (top two price levels)
        auto r_it = priceLevels.rbegin(); // Highest price
        int64_t high_delta = r_it->second.deltaAtPrice;
        if (std::next(r_it)!= priceLevels.rend()) { // Check if a second level exists
            r_it++; // Move to the second highest price
            high_delta += r_it->second.deltaAtPrice;
        }
        BAR.barHighDelta = high_delta;

        // Bar Low Delta (bottom two price levels)
        auto it = priceLevels.begin(); // Lowest price
        int64_t low_delta = it->second.deltaAtPrice;
        if (std::next(it)!= priceLevels.end()) { // Check if a second level exists
            it++; // Move to the second lowest price
            low_delta += it->second.deltaAtPrice;
        }
        BAR.barLowDelta = low_delta;
    }
    
}
```

### src/updateWeekSensitiveFeatures.cpp

This file contains the `updateWeekChangeSensitiveFeatures` function, which is responsible for updating features that are sensitive to changes in the week (e.g., at the end of a trading week).

```cpp
#include "../dataStructure.h"



void updateWeekChangeSensitiveFeatures(Contract& contract) {
    // Update week change sensitive features
    contract.weeks.back().vwap = 0.0;
    contract.weeks.back().poc = 0.0;    
    contract.weeks.back().vah = 0.0;
    contract.weeks.back().val = 0.0;
    
    
}
```

### src/indicators/BBands.cpp

This file is currently empty but is intended to contain the implementation for Bollinger Bands calculations.

```cpp



```

### src/indicators/indicators.h

This header file declares functions for various technical indicators, such as RSI and VWAP.

```cpp
#ifndef INDICATORS_H
#define INDICATORS_H

#include "../dataStructure.h"
#include <string>
#include <utility>
#include <limits>    // For std::numeric_limits::quiet_NaN




// rsi calculations
    // this will create a single columnin the day struct in the days vector of week struct
    // this will add only one result in the day.RSI
std::tuple<double, double, double> calculateRSI(Contract& contract,
    int period,
    double prevAvgGain = std::numeric_limits<double>::quiet_NaN(),
    double prevAvgLoss = std::numeric_limits<double>::quiet_NaN()){};

// vwap calculations




#endif // INDICATORS_H
```

### src/indicators/RSI.cpp

This file contains the implementation of the `calculateRSI` function, which computes the Relative Strength Index (RSI) for a given period. It supports both initialization and efficient updates.

```cpp
#include <iostream>
#include <vector>
#include <cmath>     // For std::isnan
#include <limits>    // For std::numeric_limits::quiet_NaN
#include <tuple>     // For std::tuple, std::make_tuple, std::tie
#include <iomanip>   // For std::setprecision
#include "../dataStructure.h"



//  * Calculates or updates the RSI state.
//  * This function has two modes based on the optional parameters:
//  *
//  * 1. INIT MODE: If prevAvgGain or prevAvgLoss is NaN (default),
//  * it calculates the latest RSI from the entire 'PRICES' vector.
//  * - It respects Req 1: If data is low, it uses an effectivePeriod.
//  *
//  * 2. UPDATE MODE: If prevAvgGain and prevAvgLoss are valid numbers,
//  * it performs an O(1) update using only the *last two PRICES* 
//  * in the 'PRICES' vector.
//  *
//  * @param PRICES A vector of PRICES. In UPDATE_MODE, this must
//  * contain at least 2 PRICES.
//  * @param period The desired RSI lookback period (e.g., 14).
//  * @param prevAvgGain The Average Gain from the previous bar.
//  * @param prevAvgLoss The Average Loss from the previous bar.
//  * @return RsiResult containing {new_rsi, new_avgGain, new_avgLoss}.
 
std::tuple<double, double, double> calculateRSI(
    Contract& contract,
    int period,
    double prevAvgGain = std::numeric_limits<double>::quiet_NaN(),
    double prevAvgLoss = std::numeric_limits<double>::quiet_NaN()
) {

    const double nan = std::numeric_limits<double>::quiet_NaN();
    auto& PRICES = contract.weeks.back().days.back().bars;


    // --- CASE 1: INITIALIZATION MODE ---
    // If no previous state is provided, calculate from the vector.
    if (std::isnan(prevAvgGain) || std::isnan(prevAvgLoss) || PRICES.size() < 15){
        
        // --- 1.1. Validation ---
        if (PRICES.size() < 2) {
            return std::make_tuple(nan, nan, nan);
        }

        // --- 1.2. Determine Effective Period (Req 1) ---
        int effectivePeriod = period;
        size_t dataSize = PRICES.size();
        if (dataSize < static_cast<size_t>(period) + 1) {
            effectivePeriod = dataSize - 1; // Use all available data
        }

        double avgGain = 0.0;
        double avgLoss = 0.0;

        // --- 1.3. Seeding Phase (Calculate first SMA) ---
        for (int i = 1; i <= effectivePeriod; ++i) {
            double change = static_cast<double>(PRICES[i].close) - static_cast<double>(PRICES[i - 1].close);
            if (change > 0) {
                avgGain += change;
            } else {
                avgLoss -= change;
            }
        }
        avgGain /= effectivePeriod;
        avgLoss /= effectivePeriod;

        // --- 1.4. Smoothing Phase (If we have more data) ---
        for (size_t i = static_cast<size_t>(effectivePeriod) + 1; i < dataSize; ++i) {
            double change = static_cast<double>(PRICES[i].close) - static_cast<double>(PRICES[i - 1].close);
            double currentGain = (change > 0) ? change : 0.0;
            double currentLoss = (change < 0) ? -change : 0.0;

            // Use the *original desired period* for smoothing
            avgGain = ((avgGain * (period - 1)) + currentGain) / period;
            avgLoss = ((avgLoss * (period - 1)) + currentLoss) / period;
        }

        // --- 1.5. Final RSI Calculation ---
        if (avgLoss == 0.0) {
            return std::make_tuple(100.0, avgGain, avgLoss);
        }
        double rs = avgGain / avgLoss;
        double rsi = 100.0 - (100.0 / (1.0 + rs));
        return std::make_tuple(rsi, avgGain, avgLoss);

    } 
    
    // --- CASE 2: UPDATE MODE ---
    // If a valid previous state is provided, perform O(1) update.
    else {
        
        // --- 2.1. Validation ---
        if (PRICES.size() < 2) {
            // Not enough data to get the latest change
            return std::make_tuple(nan, nan, nan);
        }

        // --- 2.2. Get Newest Price Change ---
        double newPrice = static_cast<double>(PRICES.back().close);
        double prevPrice = static_cast<double>(PRICES[PRICES.size() - 2].close);
        double change = newPrice - prevPrice;

        double currentGain = (change > 0) ? change : 0.0;
        double currentLoss = (change < 0) ? -change : 0.0;

        // --- 2.3. Apply Wilder's Smoothing (The O(1) Update) ---
        double newAvgGain = ((prevAvgGain * (period - 1)) + currentGain) / period;
        double newAvgLoss = ((prevAvgLoss * (period - 1)) + currentLoss) / period;

        // --- 2.4. Final RSI Calculation ---
        if (newAvgLoss == 0.0) {
            return std::make_tuple(100.0, newAvgGain, newAvgLoss);
        }
        double rs = newAvgGain / newAvgLoss;
        double newRsi = 100.0 - (100.0 / (1.0 + rs));
        return std::make_tuple(newRsi, newAvgGain, newAvgLoss);
    }
}
```

### src/indicators/VWAP.cpp

This file is currently empty but is intended to contain the implementation for Volume Weighted Average Price (VWAP) calculations.

```cpp



```

### src/SuppRess/InitialBalance.cpp

This file is currently empty but is intended to contain the implementation for Initial Balance calculations.

```cpp



```

### src/SuppRess/SR.h

This file is currently empty but is intended to contain declarations for Support/Resistance related functions.

```cpp



```

### src/TPO/TPO.h

This file is currently empty but is intended to contain declarations for Time Price Opportunity (TPO) related functions.

```cpp



```

### test/CMakeLists.txt

This file contains the CMake configuration for the test executable. It defines the test project and links against SQLite3.

```cmake
cmake_minimum_required(VERSION 3.10)
project(FootprintTest)

# Find SQLite3
find_package(SQLite3 REQUIRED)

add_executable(test_initialization
    testForInitialization.cpp
    ../database/sqlite.cpp
)

target_include_directories(test_initialization PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(test_initialization PRIVATE SQLite::SQLite3)
```

### test/testForInitialization.cpp

This file contains a test case for the initialization of the `Contract` data structure. It fetches the first tick from the database and verifies that the initial price-related fields are correctly set.

```cpp
#include "../dataStructure.h"
#include "../database/database.h"
#include <iostream>
#include <string>
#include <vector>

std::string db_path = "/home/sarjil/sarjil_u/C++/footprintTradingBot/testData/converted_database.db";
std::string table_name = "ESH24_tick";

// datetime column format = 


int main() {
    Date date = {2024, 3, 1}; // Year, Month, Day

    TickData tickData = fetchFirstTick(db_path, table_name, date);

    if (tickData.id == -1) {
        std::cerr << "No data found for the given date." << std::endl;
        return 1;
    }

    double initialPrice = tickData.Price;

    Bar testBar;
    Day testDay;
    Week testWeek;
    Contract testContract;

    // Initialize price-related fields in Bar
    testBar.open = initialPrice;
    testBar.close = initialPrice;
    testBar.high = initialPrice;
    testBar.low = initialPrice;
    testBar.barPOC = initialPrice;

    // Initialize price-related fields in Day
    testDay.vwap = initialPrice;
    testDay.poc = initialPrice;
    testDay.vah = initialPrice;
    testDay.val = initialPrice;
    testDay.ibHigh = initialPrice;
    testDay.ibLow = initialPrice;
    testDay.dayHigh = initialPrice;
    testDay.dayLow = initialPrice;
    testDay.dayClose = initialPrice;
    testDay.lastSwingHigh = initialPrice;
    testDay.lastSwingLow = initialPrice;
    testDay.lastHighVolumeNode = initialPrice;

    // Initialize price-related fields in Week
    testWeek.vwap = initialPrice;
    testWeek.poc = initialPrice;
    testWeek.vah = initialPrice;
    testWeek.val = initialPrice;
    testWeek.weekHigh = initialPrice;
    testWeek.weekLow = initialPrice;


    testContract.weeks.push_back(testWeek);
    testContract.weeks.back().days.push_back(testDay);
    testContract.weeks.back().days.back().bars.push_back(testBar);

    std::cout << "Contract Name: " << testContract.contractName << std::endl;
    std::cout << "Initial Price: " << initialPrice << std::endl;
    std::cout << "Bar Open: " << testContract.weeks.back().days.back().bars.back().open << std::endl;
    std::cout << "Day VWAP: " << testContract.weeks.back().days.back().vwap << std::endl;
    std::cout << "Week VWAP: " << testContract.weeks.back().vwap << std::endl;


    return 0;
}
```