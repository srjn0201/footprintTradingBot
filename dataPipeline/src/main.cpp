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

// Timezone library (e.g., Howard Hinnant's date library)
// Make sure to link this library in your CMakeLists.txt
#include "date/tz.h"

// Holds the final, clean data for a single tick.
struct ProcessedTick {
    int64_t id;
    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> dateTime;
    double price;
    int64_t askVolume;
    int64_t bidVolume;
};

// Writes the daily buffer to a partitioned Parquet file.
void write_buffer_to_parquet(
    const std::vector<ProcessedTick>& buffer, 
    const std::string& base_output_dir, 
    const date::year_month_day& ymd) 
{
    if (buffer.empty()) {
        return;
    }

    // Use date::format for safe and easy date string formatting
    std::string day_str = date::format("%Y-%m-%d", ymd);
    std::cout << "Writing " << buffer.size() << " ticks for day " << day_str << "..." << std::endl;

    // --- Arrow Builders for each column ---
    arrow::Int64Builder id_builder;
    arrow::TimestampBuilder time_builder(arrow::timestamp(arrow::TimeUnit::NANO, "UTC"), arrow::default_memory_pool());
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
        arrow::field("dateTime", arrow::timestamp(arrow::TimeUnit::NANO, "UTC")),
        arrow::field("price", arrow::float64()),
        arrow::field("askVolume", arrow::int64()),
        arrow::field("bidVolume", arrow::int64())
    });

    auto table = arrow::Table::Make(schema, {id_array, time_array, price_array, ask_array, bid_array});

    // --- Construct the hierarchical output path ---
    std::filesystem::path output_path(base_output_dir);
    output_path /= ("year=" + date::format("%Y", ymd));
    output_path /= ("month=" + date::format("%m", ymd));
    output_path /= ("day=" + date::format("%d", ymd));

    std::filesystem::create_directories(output_path);
    output_path /= "data.parquet";

    // --- Configure and write the Parquet file ---
    std::shared_ptr<arrow::io::FileOutputStream> outfile;
    PARQUET_ASSIGN_OR_THROW(outfile, arrow::io::FileOutputStream::Open(output_path.string()));

    parquet::WriterProperties::Builder properties_builder;
    properties_builder.compression(arrow::Compression::SNAPPY);
    
    // Using a larger row group size can improve read performance later
    PARQUET_THROW_NOT_OK(parquet::arrow::WriteTable(*table, arrow::default_memory_pool(), outfile, 1000000, properties_builder.build()));
}


// Processes all ticks for a SINGLE specified day from a SINGLE contract table.
void process_single_day(
    const std::string& db_path, 
    const std::string& table_name, 
    const std::string& date_to_process, // Expects "YYYY/MM/DD" format
    const std::string& output_dir) 
{
    try {
        SQLite::Database db(db_path, SQLite::OPEN_READONLY);
        
        SQLite::Statement query(db, "SELECT id, \"Date\", \"Time\", \"Close\", \"AskVolume\", \"BidVolume\" FROM " + table_name + " WHERE \"Date\" = ? ORDER BY id");
        query.bind(1, date_to_process);

        std::vector<ProcessedTick> daily_buffer;
        daily_buffer.reserve(2000000);
        
        date::year_month_day ymd_for_output;
        bool first_row = true;

        while(query.executeStep()) {
            std::string date_str = query.getColumn("Date");
            std::string time_str = query.getColumn("Time");
            std::string ist_time_str = date_str + " " + time_str;

            date::local_time<std::chrono::nanoseconds> local_time;
            std::istringstream ss(ist_time_str);
            ss >> date::parse("%Y/%m/%d %H:%M:%S", local_time);

            if (ss.fail()) {
                std::cerr << "Warning: Failed to parse timestamp: " << ist_time_str << std::endl;
                continue;
            }

            date::zoned_time<std::chrono::nanoseconds> ist_time{"Asia/Kolkata", local_time};

            auto utc_timepoint = ist_time.get_sys_time();

            date::zoned_time<std::chrono::nanoseconds> ny_time("America/New_York", utc_timepoint);
            
            ProcessedTick tick;
            tick.id = query.getColumn("id");
            tick.dateTime = utc_timepoint;
            tick.price = query.getColumn("Close");
            tick.askVolume = query.getColumn("AskVolume");
            tick.bidVolume = query.getColumn("BidVolume");

            daily_buffer.push_back(tick);

            if (first_row) {
                ymd_for_output = date::year_month_day{date::floor<date::days>(ny_time.get_local_time())};
                first_row = false;
            }
        }

        if (!daily_buffer.empty()) {
            write_buffer_to_parquet(daily_buffer, output_dir, ymd_for_output);
        }

    } catch (const std::exception& e) {
        std::cerr << "Database Error: " << e.what() << std::endl;
        exit(1); 
    }
}

int main(int argc, char* argv[]) {
    if (argc!= 5) {
        std::cerr << "Usage: " << argv[0] << " <db_path> <input_table_name> <processing_date_YYYY/MM/DD> <output_directory>" << std::endl;
        return 1;
    }

    const std::string db_path(argv[1]);
    const std::string input_table_name(argv[2]);
    const std::string processing_date(argv[3]);
    const std::string output_dir(argv[4]);

    std::cout << "Processing Date: " << processing_date << " from Table: " << input_table_name << std::endl;

    process_single_day(db_path, input_table_name, processing_date, output_dir);

    std::cout << "Processing for " << processing_date << " complete." << std::endl;

    return 0;
}