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


    // Initialize price-related fields in Week
    Week initialWeek;
    initialWeek.vwap = initialPrice;
    initialWeek.poc = initialPrice;
    initialWeek.vah = initialPrice;
    initialWeek.val = initialPrice;
    initialWeek.weekHigh = initialPrice;
    initialWeek.weekLow = initialPrice;

    // Add the initialized day to the week
    // initialWeek.days.push_back(initialDay);

    // Add the initialized week to the contract
    contract.weeks.push_back(initialWeek);

}