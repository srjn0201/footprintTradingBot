#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <filesystem>

// Database library
#include <SQLiteCpp/SQLiteCpp.h>

// Parquet/Arrow libraries
#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/writer.h>
#include <arrow/util/logging.h>

// Timezone library
#include "date/tz.h"

// Holds the final, clean data for a single tick.
struct ProcessedTick {
    int64_t id;
    std::chrono::time_point<std::chrono::system_clock> dateTime;
    double price;
    int64_t askVolume;
    int64_t bidVolume;
};

// Writes the buffer for a full table to a single Parquet file.
void write_table_to_parquet(
    const std::vector<ProcessedTick>& buffer,
    const std::string& output_path)
{
    if (buffer.empty()) {
        std::cout << "Buffer is empty, skipping write." << std::endl;
        return;
    }

    std::cout << "Writing " << buffer.size() << " total ticks to " << output_path << "..." << std::endl;

    // --- Arrow Builders for each column ---
    arrow::Int64Builder id_builder;
    arrow::TimestampBuilder time_builder(arrow::timestamp(arrow::TimeUnit::NANO), arrow::default_memory_pool());
    arrow::DoubleBuilder price_builder;
    arrow::Int64Builder ask_vol_builder;
    arrow::Int64Builder bid_vol_builder;

    // Append data from our buffer to the Arrow builders
    for (const auto& tick : buffer) {
        ARROW_CHECK_OK(id_builder.Append(tick.id));
        ARROW_CHECK_OK(time_builder.Append(tick.dateTime.time_since_epoch().count()));
        ARROW_CHECK_OK(price_builder.Append(tick.price));
        ARROW_CHECK_OK(ask_vol_builder.Append(tick.askVolume));
        ARROW_CHECK_OK(bid_vol_builder.Append(tick.bidVolume));
    }

    // Finalize the arrays
    std::shared_ptr<arrow::Array> id_array, time_array, price_array, ask_array, bid_array;
    ARROW_CHECK_OK(id_builder.Finish(&id_array));
    ARROW_CHECK_OK(time_builder.Finish(&time_array));
    ARROW_CHECK_OK(price_builder.Finish(&price_array));
    ARROW_CHECK_OK(ask_vol_builder.Finish(&ask_array));
    ARROW_CHECK_OK(bid_vol_builder.Finish(&bid_array));

    // Define the schema for the Parquet file
    auto schema = arrow::schema({
        arrow::field("id", arrow::int64()),
        arrow::field("DateTime", arrow::timestamp(arrow::TimeUnit::NANO)),
        arrow::field("Price", arrow::float64()),
        arrow::field("AskVolume", arrow::int64()),
        arrow::field("BidVolume", arrow::int64())
    });

    auto table = arrow::Table::Make(schema, {id_array, time_array, price_array, ask_array, bid_array});

    // --- Configure and write the Parquet file ---
    std::shared_ptr<arrow::io::FileOutputStream> outfile;
    PARQUET_ASSIGN_OR_THROW(outfile, arrow::io::FileOutputStream::Open(output_path));

    parquet::WriterProperties::Builder properties_builder;
    properties_builder.compression(arrow::Compression::SNAPPY);

    PARQUET_THROW_NOT_OK(parquet::arrow::WriteTable(*table, arrow::default_memory_pool(), outfile, 1000000, properties_builder.build()));
    std::cout << "Successfully wrote " << output_path << std::endl;
}

// Processes all ticks from a SINGLE contract table.
void process_table(
    SQLite::Database& db,
    const std::string& table_name,
    const std::string& output_dir)
{
    std::cout << "--- Processing table: " << table_name << " ---" << std::endl;
    try {
        SQLite::Statement query(db, "SELECT id, \"Date\", \"Time\", \"Close\", \"AskVolume\", \"BidVolume\" FROM \"" + table_name + "\" ORDER BY id");

        std::vector<ProcessedTick> table_buffer;
        // Reserve a large buffer to reduce re-allocations, can be tuned
        table_buffer.reserve(2000000);

        while(query.executeStep()) {
            std::string date_str = query.getColumn("Date");
            std::string time_str = query.getColumn("Time");
            std::string utc_time_str = date_str + " " + time_str;

            std::chrono::system_clock::time_point utc_timepoint;
            std::istringstream ss(utc_time_str);
            ss >> date::parse("%Y/%m/%d %H:%M:%S", utc_timepoint);

            if (ss.fail()) {
                std::cerr << "Warning: Failed to parse timestamp: " << utc_time_str << std::endl;
                continue;
            }

            // Convert UTC to New York local time
            date::zoned_time<std::chrono::nanoseconds> ny_time{"America/New_York", utc_timepoint};
            auto ny_local_time = ny_time.get_local_time();

            ProcessedTick tick;
            tick.id = query.getColumn("id");
            // Store a time_point representing the local New York time
            tick.dateTime = std::chrono::system_clock::time_point(ny_local_time.time_since_epoch());
            tick.price = query.getColumn("Close");
            tick.askVolume = query.getColumn("AskVolume");
            tick.bidVolume = query.getColumn("BidVolume");

            table_buffer.push_back(tick);
        }

        if (!table_buffer.empty()) {
            std::filesystem::path output_path(output_dir);
            std::filesystem::create_directories(output_path);
            output_path /= (table_name + ".parquet");
            write_table_to_parquet(table_buffer, output_path.string());
        } else {
            std::cout << "No data found in table " << table_name << ", skipping." << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error processing table " << table_name << ": " << e.what() << std::endl;
    }
}

// Gets the names of all tables in the database, excluding sqlite system tables.
std::vector<std::string> get_table_names(SQLite::Database& db) {
    std::vector<std::string> table_names;
    SQLite::Statement query(db, "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%'");
    while (query.executeStep()) {
        table_names.push_back(query.getColumn(0));
    }
    return table_names;
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <db_path> <output_directory>" << std::endl;
        return 1;
    }

    const std::string db_path(argv[1]);
    const std::string output_dir(argv[2]);

    try {
        SQLite::Database db(db_path, SQLite::OPEN_READONLY);
        std::cout << "Successfully opened database: " << db_path << std::endl;

        std::vector<std::string> table_names = get_table_names(db);
        std::cout << "Found " << table_names.size() << " tables to process." << std::endl;

        for (const auto& table_name : table_names) {
            process_table(db, table_name, output_dir);
        }

        std::cout << "\nAll processing complete." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Critical Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
