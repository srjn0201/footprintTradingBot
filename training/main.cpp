#include <iostream>
#include <string>
#include "dataStructure.h"
#include "database/json_writer.h"

extern void finalProcessing(Contract& contract);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <output_directory>" << std::endl;
        return 1;
    }

    const std::string output_dir(argv[1]);
    std::cout << "Output directory: " << output_dir << std::endl;

    try {
        std::cout << "Initializing contract structure..." << std::endl;
        Contract contract;
        contract.contractName = "ESM24_tick";  // Sample contract name

        std::cout << "Creating demo data..." << std::endl;
        
        // Add demo data using finalProcessing
        finalProcessing(contract);

        std::cout << "Demo data created with:" << std::endl;
        std::cout << "- Number of weeks: " << contract.weeks.size() << std::endl;
        for (size_t i = 0; i < contract.weeks.size(); ++i) {
            std::cout << "- Week " << i + 1 << " has " << contract.weeks[i].days.size() 
                     << " days with " << contract.weeks[i].days[0].bars.size() 
                     << " bars per day" << std::endl;
        }

        std::cout << "Writing to parquet file in: " << output_dir << std::endl;
        writeContractToJson(contract, output_dir);
        std::cout << "Successfully saved contract data to parquet file" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
}