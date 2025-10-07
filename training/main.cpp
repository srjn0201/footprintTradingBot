#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include "dataStructure.h"
#include "database/database.h"
#include "database/parquet_writer.h"

// Function to parse date string in YYYY/MM/DD format
std::chrono::system_clock::time_point parseDate(const std::string& date_str) {
    std::tm tm = {};
    std::stringstream ss(date_str);
    ss >> std::get_time(&tm, "%Y/%m/%d");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}


// Function to generate weeks from start to end date
std::vector<Week> generateWeeks(const std::chrono::system_clock::time_point& start_date, const std::chrono::system_clock::time_point& end_date) {
    std::vector<Week> weeks;
    auto current_date = start_date;

    while (current_date <= end_date) {
        Week week;
        week.weekOfTheContract = std::chrono::system_clock::to_time_t(current_date);
        
        for (int i = 0; i < 7; ++i) {
            Day day;
            day.dayOfTheWeek = std::chrono::system_clock::to_time_t(current_date);
            week.days.push_back(day);
            current_date += std::chrono::hours(24);
            if (current_date > end_date) break;
        }
        weeks.push_back(week);
    }
    return weeks;
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " <db_path> <input_table_name> <start_date_YYYY/MM/DD> <end_date_YYYY/MM/DD> <output_directory>" << std::endl;
        return 1;
    }

    const std::string db_path(argv[1]);
    const std::string input_table_name(argv[2]);
    const std::string start_date_str(argv[3]);
    const std::string end_date_str(argv[4]);
    const std::string output_dir(argv[5]);

    auto start_date = parseDate(start_date_str);
    auto end_date = parseDate(end_date_str);

    Contract contract;
    contract.contractName = "FuturesContract";
    contract.weeks = generateWeeks(start_date, end_date);

    Database db(db_path);

    for (const auto& week : contract.weeks) {
        for (const auto& day : week.days) {
            time_t current_day_time = day.dayOfTheWeek;
            char date_buf[11];
            std::strftime(date_buf, sizeof(date_buf), "%Y/%m/%d", std::localtime(&current_day_time));
            std::string current_processing_date(date_buf);

            std::cout << "Processing data for date: " << current_processing_date << std::endl;

            auto data = db.fetchData(input_table_name, current_processing_date);

            // Process the data here

        }
    }

    std::cout << "Contract object has " << contract.weeks.size() << " weeks." << std::endl;
    writeContractToParquet(contract, output_dir);

    return 0;
}