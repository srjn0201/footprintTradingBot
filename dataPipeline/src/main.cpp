#include <iostream>
#include <string>
#include <vector>
#include <chrono>


// This struct will hold a single row of clean, processed data.
struct ProcessedTick {
    int64_t id;
    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> dateTime;
    double price;
    int64_t askVolume;
    int64_t bidVolume;
};



// main now takes arguments: argc is the count, argv holds the values
int main(int argc, char* argv[]) {

//     connection string = /media/sarjil/Vol2/Data/DATABASES/E-mini_S&P_DataBase.db
//     - Input Table:       ESH24_tick
//     - Output Directory:  /mnt/data/parquet

    // We expect the program name + 3 arguments, for a total of 4.
    // argv[0] is always the name of the program itself.
    if (argc != 4) {
        // std::cerr is the standard stream for printing errors.
        std::cerr << "Usage: " << argv[0] << " <db_connection_string> <input_table_name> <output_directory>" << std::endl;
        return 1; // Return a non-zero value to indicate an error
    }

    // Convert the C-style string arguments into C++ std::string for easier use.
    const std::string db_connection_string = argv[1];
    const std::string input_table_name = argv[2];
    const std::string output_directory = argv[3];

    // Print the arguments we received to verify they were parsed correctly.
    std::cout << "C++ Processor Started with the following arguments:" << std::endl;
    std::cout << "  - Connection String: " << db_connection_string << std::endl;
    std::cout << "  - Input Table:       " << input_table_name << std::endl;
    std::cout << "  - Output Directory:  " << output_directory << std::endl;

    return 0; // Return 0 to indicate success
}