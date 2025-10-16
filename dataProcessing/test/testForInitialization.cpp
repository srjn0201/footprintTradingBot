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