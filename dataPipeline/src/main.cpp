#include <iostream>
#include <string>
#include <vector>
#include <chrono>

// NEW: Include the SQLAPI++ header
#include <SQLAPI.h>

// This struct will hold a single row of clean, processed data.
struct ProcessedTick {
    int64_t id;
    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> dateTime;
    double price;
    int64_t askVolume;
    int64_t bidVolume;
};

// --- NEW FUNCTION: Main Processing Logic ---
void process_contract_table(const std::string& db_path, const std::string& table_name) {
    SAConnection db;
    try {
    // Step 1: Connect to the SQLite database
    // SAConnection::Connect signature is: Connect(dbString, user, password, client, handler)
    // Pass empty user/password and then the client enum.
    db.Connect(SAString(db_path.c_str()), SAString(""), SAString(""), SA_SQLite_Client);
        std::cout << "Successfully connected to " << db_path << std::endl;

        SACommand cmd(&db);
        const long CHUNK_SIZE = 1000000; // 1 million rows
        long offset = 0;
        int total_rows_processed = 0;

        // Step 2: Loop to fetch data in chunks
        while (true) {
            // Use :1 and :2 as placeholders for our parameters
            SAString query = SAString("SELECT id, date, time, close, askVolume, bidVolume FROM ") + 
                           SAString(table_name.c_str()) + 
                           SAString(" ORDER BY id LIMIT :1 OFFSET :2");
            cmd.setCommandText(query);
            
            // Bind the chunk size and offset to the placeholders
            cmd.Param(1).setAsInt64() = CHUNK_SIZE;
            cmd.Param(2).setAsInt64() = offset;

            cmd.Execute();
            
            int rows_in_chunk = 0;
            // Step 3: Iterate over the rows in the current chunk
            while(cmd.FetchNext()) {
                rows_in_chunk++;
                // For now, we just print the data to verify it's being read.
                // In the next step, we'll do the real transformation here.
                std::cout << "ID: " << cmd.Field("id").asInt64()
                          << ", Date: " << cmd.Field("date").asString().GetMultiByteChars()
                          << ", Close: " << cmd.Field("close").asDouble() << std::endl;
            }

            if (rows_in_chunk == 0) {
                // No more rows left, so we can exit the loop.
                break;
            }

            total_rows_processed += rows_in_chunk;
            std::cout << "--- Processed chunk of " << rows_in_chunk << " rows. Total processed: " << total_rows_processed << " ---" << std::endl;

            // Prepare for the next chunk
            offset += CHUNK_SIZE;
        }

        db.Disconnect();
        std::cout << "Finished processing. Total rows: " << total_rows_processed << std::endl;

    } catch(const SAException& e) {
        // SQLAPI++ throws exceptions on errors
        std::cerr << "Database Error: " << e.ErrText().GetMultiByteChars() << std::endl;
        if (db.isConnected()) {
            db.Disconnect();
        }
    }
}


// The main function now just calls our processing function
int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <db_path> <input_table_name> <output_directory>" << std::endl;
        return 1;
    }

    const std::string db_path = argv[1];
    const std::string input_table_name = argv[2];
    const std::string output_directory = argv[3];

    // Call the main processing function with the database path and table name
    process_contract_table(db_path, input_table_name);

    return 0;
}