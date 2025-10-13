#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "dataStructure.h"
#include "database/json_writer.h"
#include "convertDatesToWeek.cpp"
#include "database/database.h"
#include "database/json_writer.h"


extern void finalProcessing(int bar_range, Contract& contract , std::vector<weekVector>& weeksVector, const std::string& database_path, const std::string& table_name);

//test usage: ./footprint_trainer <database_path> <table_name> <start_date> <end_date> <output_directory>
//test usage: ./footprint_trainer "/home/sarjil/sarjil_u/C++/footprintTradingBot/testData/converted_database.db" ESH24_tick 2024-02-13 2024-03-13 "/media/sarjil/Vol2/Data/dataFromFootprint/"

int main(int argc, char* argv[]) {
    if (argc != 7) {
        std::cerr << "Usage: " << argv[0] << " <bar_range> <database_path> <table_name> <start_date> <end_date> <output_directory>" << std::endl;
        return 1;
    }
    const int bar_range = std::stoi(argv[1]);
    const std::string database_path(argv[2]);
    const std::string table_name(argv[3]);
    const std::string start_date(argv[4]);
    const std::string end_date(argv[5]);
    const std::string output_dir(argv[6]);

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
    contract.contractName = table_name;
    // Signal signalData;

    // convert date range to weeksVector
    auto weeksVector = convertDatesToWeeks(startDate, endDate);
    // for (const auto& week : weeksVector) {
    //     std::cout << "Week " << week.weekNumber << "\n";
    //     for (const auto& day : week.days) {
    //         if (day.dayNumber != -1) {
    //             std::cout << "  Day " << day.dayNumber << ": "
    //                      << day.date.y << "-"
    //                      << std::setfill('0') << std::setw(2) << day.date.m << "-"
    //                      << std::setfill('0') << std::setw(2) << day.date.d << "\n";
    //         }
    //     }
    //     std::cout << "\n";
    // }



//-----------------------------------------------------------------------------------------------------------------

    // call the finalProcessing function which accepts the contract by reference 
    finalProcessing(bar_range,contract, weeksVector, database_path,  table_name);
//     and the weeksVector by reference and database path and table name and also signal structure by reference
    
    
    
    // then final processed contract structure will be saved as json file using the writeContractToJson function from json_writer.h and save it to provided output directory path
    writeContractToJson(contract, output_dir);


    // then save the signal structure in the signal database and also in the csv format to the provided output directory path
    
    return 0;
}