#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <ctime>
#include <filesystem>

#include <SQLAPI.h> 

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/writer.h>
// --- THE FIX: This header defines ARROW_CHECK_OK ---
#include <arrow/util/logging.h>

struct ProcessedTick {
    int64_t id;
    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> dateTime;
    double price;
    int64_t askVolume;
    int64_t bidVolume;
};

void write_buffer_to_parquet(
    const std::vector<ProcessedTick>& buffer, 
    const std::string& base_output_dir, 
    const std::string& day_str) 
{
    if (buffer.empty()) {
        return;
    }
    std::cout << "Writing " << buffer.size() << " ticks for day " << day_str << "..." << std::endl;

    arrow::Int64Builder id_builder;
    arrow::TimestampBuilder time_builder(arrow::timestamp(arrow::TimeUnit::NANO), arrow::default_memory_pool());
    arrow::DoubleBuilder price_builder;
    arrow::Int64Builder ask_vol_builder;
    arrow::Int64Builder bid_vol_builder;

    for (const auto& tick : buffer) {
        ARROW_CHECK_OK(id_builder.Append(tick.id));
        ARROW_CHECK_OK(time_builder.Append(tick.dateTime.time_since_epoch().count()));
        ARROW_CHECK_OK(price_builder.Append(tick.price));
        ARROW_CHECK_OK(ask_vol_builder.Append(tick.askVolume));
        ARROW_CHECK_OK(bid_vol_builder.Append(tick.bidVolume));
    }

    std::shared_ptr<arrow::Array> id_array, time_array, price_array, ask_array, bid_array;
    ARROW_CHECK_OK(id_builder.Finish(&id_array));
    ARROW_CHECK_OK(time_builder.Finish(&time_array));
    ARROW_CHECK_OK(price_builder.Finish(&price_array));
    ARROW_CHECK_OK(ask_vol_builder.Finish(&ask_array));
    ARROW_CHECK_OK(bid_vol_builder.Finish(&bid_array));

    auto schema = arrow::schema({
        arrow::field("id", arrow::int64()),
        arrow::field("DateTime", arrow::timestamp(arrow::TimeUnit::NANO)),
        arrow::field("ClosePrice", arrow::float64()),
        arrow::field("askVolume", arrow::int64()),
        arrow::field("bidVolume", arrow::int64())
    });

    auto table = arrow::Table::Make(schema, {id_array, time_array, price_array, ask_array, bid_array});

    std::string year = day_str.substr(0, 4);
    std::string month = day_str.substr(5, 2);
    std::string day = day_str.substr(8, 2);

    std::filesystem::path output_path(base_output_dir);
    output_path /= ("year=" + year);
    output_path /= ("month=" + month);
    output_path /= ("day=" + day);

    std::filesystem::create_directories(output_path);

    output_path /= "data.parquet";

    std::shared_ptr<arrow::io::FileOutputStream> outfile;
    PARQUET_ASSIGN_OR_THROW(outfile, arrow::io::FileOutputStream::Open(output_path.string()));

    parquet::WriterProperties::Builder properties_builder;
    properties_builder.compression(arrow::Compression::SNAPPY);

    PARQUET_THROW_NOT_OK(parquet::arrow::WriteTable(*table, arrow::default_memory_pool(), outfile, 1000000, properties_builder.build()));
}

// ... The rest of the file (process_contract_table and main) is unchanged ...
void process_contract_table(const std::string& db_path, const std::string& table_name, const std::string& output_dir) {
    SAConnection db;
    try {
        db.Connect(db_path.c_str(), "", "", SA_SQLite_Client);
        std::cout << "Successfully connected to " << db_path << std::endl;
        std::cout << "Reading and processing rows..." << std::endl;

        std::vector<ProcessedTick> daily_buffer;
        std::string current_processing_day = "";

        SACommand cmd(&db);
        const long CHUNK_SIZE = 5000000;
        long offset = 0;
        int total_rows_processed = 0;

        while (true) {
            std::string sql_query = "SELECT id, date, time, close, askVolume, bidVolume FROM " + table_name + " ORDER BY id LIMIT :1 OFFSET :2";
            cmd.setCommandText(sql_query.c_str());

            cmd.Param(1).setAsInt64() = CHUNK_SIZE;
            cmd.Param(2).setAsInt64() = offset;
            cmd.Execute();

            int rows_in_chunk = 0;
            while(cmd.FetchNext()) {
                rows_in_chunk++;

                std::string date_str = cmd.Field("date").asString().GetMultiByteChars();
                std::string time_str = cmd.Field("time").asString().GetMultiByteChars();
                std::string ist_time_str = date_str + " " + time_str;

                std::tm tm = {};
                std::istringstream ss(ist_time_str);
                ss >> std::get_time(&tm, "%Y/%m/%d %H:%M:%S");

                if (ss.fail()) { continue; }

                time_t as_time_t = std::mktime(&tm);
                auto ist_timepoint = std::chrono::system_clock::from_time_t(as_time_t);

                const auto IST_OFFSET = std::chrono::hours(5) + std::chrono::minutes(30);
                auto utc_timepoint = ist_timepoint - IST_OFFSET;

                const std::chrono::hours NY_OFFSET(4); // WARNING: Does not handle DST
                auto ny_timepoint = utc_timepoint - NY_OFFSET;

                time_t ny_t = std::chrono::system_clock::to_time_t(ny_timepoint);
                char date_buffer[11]; // YYYY-MM-DD\0
                std::strftime(date_buffer, sizeof(date_buffer), "%Y-%m-%d", std::gmtime(&ny_t));
                std::string tick_day_str(date_buffer);

                if (current_processing_day.empty()) {
                    current_processing_day = tick_day_str;
                } else if (tick_day_str != current_processing_day) {
                    write_buffer_to_parquet(daily_buffer, output_dir, current_processing_day);
                    daily_buffer.clear();
                    current_processing_day = tick_day_str;
                }

                ProcessedTick tick;
                tick.id = cmd.Field("id").asInt64();
                tick.dateTime = ny_timepoint;
                tick.price = cmd.Field("close").asDouble();
                tick.askVolume = cmd.Field("askVolume").asInt64();
                tick.bidVolume = cmd.Field("bidVolume").asInt64();

                daily_buffer.push_back(tick);
            }

            if (rows_in_chunk == 0) {
                break;
            }
            total_rows_processed += rows_in_chunk;
            offset += CHUNK_SIZE;
        }

        if (!daily_buffer.empty()) {
            write_buffer_to_parquet(daily_buffer, output_dir, current_processing_day);
        }

        db.Disconnect();
        std::cout << "Finished processing. Total rows read: " << total_rows_processed << std::endl;

    } catch(const SAException& e) {
        std::cerr << "Database Error: " << e.ErrText().GetMultiByteChars() << std::endl;
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <db_path> <input_table_name> <output_directory (optional)>" << std::endl;
        return 1;
    }

    const std::string db_path = argv[1];
    const std::string input_table_name = argv[2];

    std::filesystem::path fs_db_path(db_path);
    std::string output_dir = argv[3];
    if (output_dir.empty()) {
        output_dir = fs_db_path.parent_path().string();
    }


    std::cout << "Output directory set to: " << output_dir << std::endl;

    process_contract_table(db_path, input_table_name, output_dir);

    return 0;
}