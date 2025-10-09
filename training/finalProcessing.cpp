#include "dataStructure.h"
#include <vector>
#include <numeric>
#include <random>
#include <chrono>
#include <ctime>
#include <iostream>


// Helper function to generate random double within a range
double randomDouble(double min, double max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

// Helper function to create demo price levels for footprint
Footprint createDemoFootprint() {
    Footprint fp;
    // Create 5 price levels
    double basePrice = 4000.0;  // Example base price for ES
    for(int i = 0; i < 5; i++) {
        double price = basePrice + (i * 0.25);  // 0.25 point intervals
        PriceLevel level;
        level.bidVolume = randomDouble(100, 500);
        level.askVolume = randomDouble(100, 500);
        level.isBuyImbalance = level.bidVolume > level.askVolume * 1.5;
        level.isSellImbalance = level.askVolume > level.bidVolume * 1.5;
        fp.priceLevels[price] = level;
    }
    return fp;
}

// Create a demo bar with realistic-looking data
Bar createDemoBar(time_t startTime) {
    Bar bar;
    
    // Time settings
    bar.startTime = startTime;
    bar.endTime = startTime + 300;  // 5-minute bars
    
    // OHLCV data
    bar.open = randomDouble(4000.0, 4010.0);
    bar.high = bar.open + randomDouble(0.5, 2.0);
    bar.low = bar.open - randomDouble(0.5, 2.0);
    bar.close = randomDouble(bar.low, bar.high);
    bar.barTotalVolume = randomDouble(5000, 10000);
    
    // Footprint data
    bar.footprint = createDemoFootprint();
    
    // Calculate imbalances and delta
    bar.imbalanceCount = static_cast<int>(randomDouble(0, 5));
    bar.delta = static_cast<int>(randomDouble(-1000, 1000));
    bar.barDeltaChange = static_cast<int>(randomDouble(-500, 500));
    bar.barHighDelta = static_cast<int>(randomDouble(0, 200));
    bar.barLowDelta = static_cast<int>(randomDouble(-200, 0));
    bar.barPOC = bar.open + randomDouble(-0.5, 0.5);
    
    // Signal data
    bar.signal = static_cast<int>(randomDouble(0, 2));
    bar.signalID = static_cast<int>(randomDouble(0, 100));
    bar.signalStatus = randomDouble(0, 1) > 0.5;
    
    // Price differences
    bar.priceCurrentDayVwapDiff = randomDouble(-2.0, 2.0);
    bar.priceCurrentDayVwapUpperStdDevDiff = randomDouble(0.5, 2.0);
    bar.priceCurrentDayVwapLowerStdDevDiff = randomDouble(-2.0, -0.5);
    bar.pricePreviousDayVwapDiff = randomDouble(-3.0, 3.0);
    bar.priceWeeklyVwapDiff = randomDouble(-5.0, 5.0);
    
    // More price differences
    bar.priceBBandUpperDiff = randomDouble(1.0, 3.0);
    bar.priceBBandLowerDiff = randomDouble(-3.0, -1.0);
    bar.PriceBBandMiddleDiff = randomDouble(-1.0, 1.0);
    
    // TPO differences
    bar.isPriceInVA = randomDouble(0, 1) > 0.3;
    bar.priceCurrDayVAHDiff = randomDouble(0.5, 2.0);
    bar.priceCurrDayVALDiff = randomDouble(-2.0, -0.5);
    bar.pricePrevDayPOCDiff = randomDouble(-1.0, 1.0);
    bar.pricePrevDayVAHDiff = randomDouble(1.0, 3.0);
    bar.pricePrevDayVALDiff = randomDouble(-3.0, -1.0);
    
    // Support/Resistance differences
    bar.priceIBHighDiff = randomDouble(2.0, 4.0);
    bar.priceIBLowDiff = randomDouble(-4.0, -2.0);
    bar.pricePrevDayHighDiff = randomDouble(3.0, 6.0);
    bar.pricePrevDayLowDiff = randomDouble(-6.0, -3.0);
    bar.pricePrevDayCloseDiff = randomDouble(-2.0, 2.0);
    bar.priceWeekHighDiff = randomDouble(5.0, 10.0);
    bar.priceWeekLowDiff = randomDouble(-10.0, -5.0);
    
    // Technical levels
    bar.priceLastSwingHighDiff = randomDouble(2.0, 5.0);
    bar.priceLastSwingLowDiff = randomDouble(-5.0, -2.0);
    bar.priceLastHVNDiff = randomDouble(-3.0, 3.0);
    
    return bar;
}

// Create a demo day with the specified number of bars
Day createDemoDay(time_t dayStart, int numBars) {
    Day day;
    day.dayOfTheWeek = dayStart;
    
    // Add bars
    time_t barStart = dayStart + 34200; // Start at 9:30 AM
    for(int i = 0; i < numBars; i++) {
        day.bars.push_back(createDemoBar(barStart));
        barStart += 300; // Next 5-minute bar
    }
    
    // Set day-level calculations
    day.deltaZscore20bars = randomDouble(-2.0, 2.0);
    day.cumDelta5barSlope = randomDouble(-1.0, 1.0);
    day.priceCumDeltaDivergence5bar = randomDouble(-2.0, 2.0);
    day.priceCumDeltaDivergence10bar = randomDouble(-3.0, 3.0);
    day.interactionReversal20bar = randomDouble(-1.0, 1.0);
    
    // Technical indicators
    day.vwap = randomDouble(4000.0, 4010.0);
    day.vwapUpperStdDev = day.vwap + randomDouble(2.0, 4.0);
    day.vwapLowerStdDev = day.vwap - randomDouble(2.0, 4.0);
    day.bbMiddle = day.vwap;
    day.bbUpper = day.vwap + randomDouble(5.0, 8.0);
    day.bbLower = day.vwap - randomDouble(5.0, 8.0);
    day.BBandWidth = randomDouble(10.0, 20.0);
    day.rsi = randomDouble(30.0, 70.0);
    day.poc = day.vwap + randomDouble(-2.0, 2.0);
    day.vah = day.poc + randomDouble(3.0, 6.0);
    day.val = day.poc - randomDouble(3.0, 6.0);
    
    // Other day statistics
    day.ibHigh = day.vwap + randomDouble(4.0, 8.0);
    day.ibLow = day.vwap - randomDouble(4.0, 8.0);
    day.dayHigh = day.ibHigh + randomDouble(0.0, 4.0);
    day.dayLow = day.ibLow - randomDouble(0.0, 4.0);
    day.dayClose = randomDouble(day.dayLow, day.dayHigh);
    
    // Volume statistics
    day.totalVolume = randomDouble(100000, 200000);
    day.cumulativeDelta = randomDouble(-5000, 5000);
    day.lastSwingHigh = day.dayHigh - randomDouble(0.0, 2.0);
    day.lastSwingLow = day.dayLow + randomDouble(0.0, 2.0);
    day.lastHighVolumeNode = day.poc + randomDouble(-2.0, 2.0);
    
    return day;
}

// Create a demo week with the specified number of days
Week createDemoWeek(time_t weekStart, int numDays) {
    Week week;
    week.weekOfTheContract = weekStart;
    
    // Add days
    for(int i = 0; i < numDays; i++) {
        time_t dayStart = weekStart + (i * 86400); // 86400 seconds per day
        week.days.push_back(createDemoDay(dayStart, 10)); // 10 bars per day
    }
    
    // Set week-level calculations
    week.vwap = randomDouble(4000.0, 4010.0);
    week.poc = week.vwap + randomDouble(-2.0, 2.0);
    week.vah = week.poc + randomDouble(5.0, 10.0);
    week.val = week.poc - randomDouble(5.0, 10.0);
    week.weekHigh = week.vah + randomDouble(0.0, 5.0);
    week.weekLow = week.val - randomDouble(0.0, 5.0);
    
    return week;
}

void finalProcessing(Contract& contract) {
    // Get current time for the first week
    time_t currentTime = std::time(nullptr);
    
    // Create first week (5 days)
    std::cout << "Creating first week with 5 days..." << std::endl;
    Week firstWeek = createDemoWeek(currentTime, 5);
    contract.weeks.push_back(firstWeek);
    
    // Create second week (3 days), starting 7 days after first week
    std::cout << "Creating second week with 3 days..." << std::endl;
    time_t nextWeekStart = currentTime + (7 * 86400);
    Week secondWeek = createDemoWeek(nextWeekStart, 3);
    contract.weeks.push_back(secondWeek);
    
    std::cout << "Demo data generation complete." << std::endl;
}
