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
//test usage: ./footprint_trainer 2.5 "/Users/sarjil/sarjil/main/footprintTradingBot/testData/converted_database.db" ESH24_tick 2024-02-13 2024-03-13 "/Users/sarjil/sarjil/main/footprintTradingBot/testData/testOutputData/" 3.0

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