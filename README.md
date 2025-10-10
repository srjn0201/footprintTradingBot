# Footprint Trading Bot

This README file provides a detailed explanation of the source code for the Footprint Trading Bot project.

## `dataPipeline/src/main.cpp`

```cpp
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
#include <arrow/api.hh>
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
```

### Description

This C++ file is the main executable for the data pipeline. Its primary responsibility is to read tick data from a SQLite database, process it, and write it to a partitioned Parquet file.

### Logic

1.  **`main` function**:
    *   Parses command-line arguments: database path, input table name, processing date, and output directory.
    *   Calls the `process_single_day` function to handle the data processing for the specified date.

2.  **`process_single_day` function**:
    *   Opens the SQLite database in read-only mode.
    *   Constructs a SQL query to select tick data for a specific date from the specified table.
    *   Iterates through the query results, creating a `ProcessedTick` struct for each row.
    *   The timestamp, originally in IST (Asia/Kolkata), is parsed and converted to UTC. This is crucial for maintaining a consistent time reference across all data.
    *   The UTC time is then used to determine the correct date in the "America/New_York" timezone, which is used for partitioning the data.
    *   The processed ticks are stored in a `daily_buffer`.
    *   After processing all ticks for the day, it calls `write_buffer_to_parquet` to write the data to a Parquet file.

3.  **`write_buffer_to_parquet` function**:
    *   This function takes the buffered tick data and writes it to a Parquet file.
    *   It uses Apache Arrow builders (`Int64Builder`, `TimestampBuilder`, `DoubleBuilder`) to construct the columns of the Parquet file in memory. This is an efficient way to build the column data.
    *   A schema is defined to describe the structure of the data (column names and types).
    *   An Arrow `Table` is created from the schema and the arrays of data.
    *   The output path is constructed in a hierarchical format (`year=.../month=.../day=...`). This is a common and effective convention for partitioning data, as it allows for efficient querying by date.
    *   Finally, the `parquet::arrow::WriteTable` function writes the Arrow `Table` to a Snappy-compressed Parquet file. Snappy compression provides a good balance between compression ratio and speed.

### Explanation

The purpose of this code is to create a robust and efficient data pipeline for processing financial tick data.

*   **SQLite to Parquet**: The data is moved from a transactional database (SQLite) to a columnar storage format (Parquet). Parquet is highly optimized for analytical queries, which is exactly what is needed for training a trading bot.
*   **Timezone Handling**: Correctly handling timezones is critical in financial applications. This code demonstrates the proper way to parse a local time, convert it to a universal standard (UTC), and then determine the correct "business date" in a different timezone (New York). This prevents ambiguity and errors related to time.
*   **Partitioning**: The data is partitioned by year, month, and day. This is a key optimization for a data lake architecture. It allows for very fast data retrieval when you need to query data for a specific date range, as you can skip reading the data for other dates entirely.
*   **Efficiency**: The use of Apache Arrow and its builders, along with reserving memory for the `daily_buffer`, are all techniques to make the data processing as fast and memory-efficient as possible.

## `pythonChecker/main.py`

```python
import pandas as pd

path = '/media/sarjil/Vol2/Data/contract.parquet'
# path = '/media/sarjil/Vol2/Data/parquetData-esMINI/year=2024/month=01/day=03/data.parquet'

data = pd.read_parquet(path)

print(data.tail())
```

### Description

This Python script is a simple utility for checking the contents of a Parquet file.

### Logic

1.  **Import pandas**: Imports the pandas library, which is essential for working with DataFrames.
2.  **Define Path**: A path to a Parquet file is defined. There are two paths, one of which is commented out. This allows for easily switching between two different files for checking.
3.  **Read Parquet**: The `pd.read_parquet()` function is used to read the Parquet file into a pandas DataFrame.
4.  **Print Tail**: The `.tail()` method of the DataFrame is called to print the last 5 rows of the data. This is a quick way to verify that the file has been read correctly and to inspect the most recent data.

### Explanation

This script serves as a quick and easy way to manually inspect the data in the Parquet files. After the C++ data pipeline has run, you can use this script to quickly verify that the output is correct without having to write a lot of boilerplate code. It's a common practice in data engineering to have small scripts like this for debugging and verification purposes.

## `pythonManager/main.py`

```python
# main.py (previously run_pipeline.py)

import os
import subprocess
import logging
from datetime import datetime
import pandas as pd
from sqlalchemy import create_engine, inspect
from tqdm import tqdm

# ==============================================================================
# --- CONFIGURATION ---
# --- (Modify these paths to match your system) ---
# ==============================================================================

# Full path to your SQLite database file.
# Example for Windows: "C:\\Users\\YourUser\\Documents\\trading_data\\tick_data.db"
# Example for Linux/macOS: "/home/sarjil/sarjil_u/C++/footprintTradingBot/data/ES_tick_data.db"
DB_PATH = "/media/sarjil/Vol2/Data/DATABASES/E-mini_S&P_DataBase.db"

# Full path to your compiled C++ data cleaner executable.
# Example for Windows: "C:\\Users\\YourUser\\Documents\\trading_bot\\build\\Release\\data_cleaner.exe"
# Example for Linux/macOS: "/home/sarjil/sarjil_u/C++/footprintTradingBot/data_cleaner/build/data_cleaner"
CLEANER_EXECUTABLE_PATH = "/home/sarjil/sarjil_u/C++/footprintTradingBot/dataPipeline/build/data_cleaner"

# Root directory where the partitioned Parquet files will be saved.
# The script will create this directory if it doesn't exist.
OUTPUT_DIR = "/media/sarjil/Vol2/Data/parquetData-esMINI/"

# ==============================================================================
# --- END CONFIGURATION ---
# ==============================================================================


def setup_logging():
    """Configures a professional logging format for the script."""
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(levelname)s - %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S'
    )

def get_contract_tables(engine):
    """
    Scans the database and returns a list of table names that appear to be
    futures contracts (e.g., starting with 'ES_').
    """
    logging.info("Scanning database for contract tables...")
    try:
        inspector = inspect(engine)
        all_tables = inspector.get_table_names()
        logging.info(f"Found tables: {all_tables}")
        
        # --- THIS IS THE FIX ---
        # This list comprehension filters for tables starting with 'ES_' (case-insensitive).
        # Adjust the 'ES_' prefix if your tables are named differently.
        contract_tables = [t for t in all_tables if t.upper().startswith('ES') and t.upper().endswith('_TICK')]
        
        if not contract_tables:
            logging.error("No contract tables found in the database. Please check DB_PATH and table names.")
            return None
            
        logging.info(f"Found {len(contract_tables)} potential contract tables: {contract_tables}")
        return contract_tables
    except Exception as e:
        logging.error(f"Failed to connect to the database and inspect tables: {e}")
        return None

def build_roll_map(engine, contract_tables):
    """
    PASS 1: Scans all contract tables to determine the most liquid contract for each day.
    
    Returns a dictionary mapping each date (in 'YYYY/MM/DD' format) to the name 
    of the table that had the highest volume on that day.
    """
    logging.info("--- Starting Pass 1: Building the Roll Map ---")
    logging.info("Querying daily volumes for all contracts. This may take a few minutes...")

    all_daily_volumes = []

    with engine.connect() as connection:
        # Get columns of the first table to help debugging
        try:
            first_table = contract_tables[0]
            columns = pd.read_sql(f'SELECT * FROM "{first_table}" LIMIT 1', connection).columns
            logging.info(f"Columns in table {first_table}: {columns.tolist()}")
        except Exception as e:
            logging.error(f"Failed to get columns for table {first_table}: {e}")

        for table_name in tqdm(contract_tables, desc="Scanning Contracts", unit="table"):
            try:
                # This query is fast because the DB engine does the aggregation.
                # It only returns a few hundred rows per table, not millions.
                query = f'SELECT "Date", SUM("Volume") as daily_volume FROM "{table_name}" GROUP BY "Date";'
                daily_df = pd.read_sql(query, connection)
                daily_df['contract_name'] = table_name
                all_daily_volumes.append(daily_df)
            except Exception as e:
                logging.error(f"Failed to query daily volume for table {table_name}: {e}")
                continue

    if not all_daily_volumes:
        logging.error("Could not retrieve any daily volume data. Aborting.")
        return None

    master_volumes_df = pd.concat(all_daily_volumes, ignore_index=True)
    
    # For each date, find the row (and thus the contract) with the maximum volume.
    idx = master_volumes_df.groupby('Date')['daily_volume'].idxmax()
    dominant_contracts_df = master_volumes_df.loc[idx]
    
    # Create the final map: { 'YYYY/MM/DD': 'table_name' }
    # This format is what our C++ program expects for the date.
    roll_map = pd.Series(
        dominant_contracts_df.contract_name.values, 
        index=dominant_contracts_df.Date
    ).to_dict()

    logging.info(f"Roll map successfully built for {len(roll_map)} unique trading days.")
    logging.info("--- Pass 1 Complete ---")
    print(roll_map)
    return roll_map

def run_processing(roll_map, db_path, cleaner_executable_path, output_dir):
    """
    PASS 2: Iterates through the roll map and calls the C++ processor for each day.
    """
    logging.info("--- Starting Pass 2: Processing Ticks with C++ Engine ---")

    if not os.path.exists(cleaner_executable_path):
        logging.error(f"C++ executable not found at: {cleaner_executable_path}")
        logging.error("Please check the CLEANER_EXECUTABLE_PATH variable and ensure the program is compiled.")
        return

    # Create the main output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)

    # Sort the dates to process them chronologically
    sorted_dates = sorted(roll_map.keys())

    for date_str in tqdm(sorted_dates, desc="Processing Days", unit="day"):
        contract_name = roll_map[date_str]
        
        # Construct the command to execute for the C++ program
        command = [
            cleaner_executable_path,
            db_path,
            contract_name,
            date_str,
            output_dir
        ]
        
        try:
            # Execute the C++ program.
            # `check=True` will automatically raise an exception if the C++ program fails (returns a non-zero exit code).
            result = subprocess.run(
                command,
                capture_output=True,
                text=True,
                check=True 
            )
            # Log stdout from the C++ program for debugging if needed
            if result.stdout:
                # We'll log this at a DEBUG level to avoid cluttering the main log
                logging.debug(result.stdout.strip())

        except FileNotFoundError:
            logging.error(f"FATAL: The command '{cleaner_executable_path}' was not found.")
            break
        except subprocess.CalledProcessError as e:
            logging.error(f"FATAL: C++ process failed while processing {date_str} from table {contract_name}.")
            logging.error(f"C++ process returned exit code: {e.returncode}")
            logging.error(f"--- C++ STDOUT ---:\n{e.stdout}")
            logging.error(f"--- C++ STDERR ---:\n{e.stderr}")
            break
        except Exception as e:
            logging.error(f"An unexpected Python error occurred while running the C++ process for {date_str}: {e}")
            break
            
    logging.info("--- Pass 2 Complete ---")


def main():
    """Main function to orchestrate the data cleaning pipeline."""
    setup_logging()
    start_time = datetime.now()
    logging.info("Starting the continuous contract creation pipeline...")

    # Create the database engine using SQLAlchemy
    try:
        # For a SQLite, the URI format is sqlite:/// followed by the absolute path.
        db_uri = f"sqlite:///{os.path.abspath(DB_PATH)}"
        engine = create_engine(db_uri)
    except Exception as e:
        logging.error(f"Failed to create database engine for '{DB_PATH}': {e}")
        return

    # Get list of tables to process
    contract_tables = get_contract_tables(engine)
    if not contract_tables:
        return

    # Pass 1: Build the roll map
    roll_map = build_roll_map(engine, contract_tables)
    if not roll_map:
        return

    # Pass 2: Process the data using the C++ engine
    run_processing(roll_map, DB_PATH, CLEANER_EXECUTABLE_PATH, OUTPUT_DIR)

    end_time = datetime.now()
    logging.info(f"Pipeline finished. Total time taken: {end_time - start_time}")


if __name__ == "__main__":
    main()
```

### Description

This Python script is the main orchestrator for the entire data processing pipeline. It's responsible for figuring out which data to process for each day and then invoking the high-performance C++ application to do the actual processing.

### Logic

This script operates in two main passes:

1.  **Pass 1: Building the Roll Map (`build_roll_map`)**
    *   **Problem**: Futures contracts expire. To create a continuous data series for backtesting, you need to "roll" from one contract to the next. The most common way to do this is to use the contract with the highest trading volume (the most "liquid" contract) for any given day.
    *   **Solution**: This function scans all the contract tables in the SQLite database. For each table, it calculates the total volume for each day. It then aggregates all this information and, for each day, identifies which contract had the highest volume. The result is a `roll_map`, which is a dictionary that maps each date to the name of the most liquid contract for that date.

2.  **Pass 2: Processing the Ticks (`run_processing`)**
    *   **Problem**: Now that we know which contract to use for each day, we need to process the tick data for that contract and day.
    *   **Solution**: This function iterates through the `roll_map`. For each date, it constructs a command-line call to the C++ `data_cleaner` executable. It passes the database path, the correct contract table name for that day, the date to process, and the output directory. It then uses Python's `subprocess.run` to execute the C++ program.

### Explanation

This script is a perfect example of using the right tool for the right job. Python, with its rich data analysis libraries (pandas, SQLAlchemy), is excellent for the high-level orchestration and analysis needed to build the roll map. However, processing millions of ticks is a computationally intensive task where C++ excels. This script acts as a "manager", figuring out the high-level logic and then delegating the heavy lifting to the compiled C++ application. This hybrid approach gives you the best of both worlds: Python's ease of use and C++'s raw performance.

## `training/finalProcessing.cpp`

```cpp
#include "dataStructure.h"
#include <vector>
#include <numeric>
#include <random>
#include <chrono>
#include <ctime>
#include <iostream>


// Helper function to generate random double within a range
double randomDouble(double min, double max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

// Helper function to create demo price levels for footprint
Footprint createDemoFootprint() {
    Footprint fp;
    // Create 5 price levels
    double basePrice = 4000.0;  // Example base price for ES
    for(int i = 0; i < 5; i++) {
        double price = basePrice + (i * 0.25);  // 0.25 point intervals
        PriceLevel level;
        level.bidVolume = randomDouble(100, 500);
        level.askVolume = randomDouble(100, 500);
        level.isBuyImbalance = level.bidVolume > level.askVolume * 1.5;
        level.isSellImbalance = level.askVolume > level.bidVolume * 1.5;
        fp.priceLevels[price] = level;
    }
    return fp;
}

// Create a demo bar with realistic-looking data
Bar createDemoBar(time_t startTime) {
    Bar bar;
    
    // Time settings
    bar.startTime = startTime;
    bar.endTime = startTime + 300;  // 5-minute bars
    
    // OHLCV data
    bar.open = randomDouble(4000.0, 4010.0);
    bar.high = bar.open + randomDouble(0.5, 2.0);
    bar.low = bar.open - randomDouble(0.5, 2.0);
    bar.close = randomDouble(bar.low, bar.high);
    bar.barTotalVolume = randomDouble(5000, 10000);
    
    // Footprint data
    bar.footprint = createDemoFootprint();
    
    // Calculate imbalances and delta
    bar.imbalanceCount = static_cast<int>(randomDouble(0, 5));
    bar.delta = static_cast<int>(randomDouble(-1000, 1000));
    bar.barDeltaChange = static_cast<int>(randomDouble(-500, 500));
    bar.barHighDelta = static_cast<int>(randomDouble(0, 200));
    bar.barLowDelta = static_cast<int>(randomDouble(-200, 0));
    bar.barPOC = bar.open + randomDouble(-0.5, 0.5);
    
    // Signal data
    bar.signal = static_cast<int>(randomDouble(0, 2));
    bar.signalID = static_cast<int>(randomDouble(0, 100));
    bar.signalStatus = randomDouble(0, 1) > 0.5;
    
    // Price differences
    bar.priceCurrentDayVwapDiff = randomDouble(-2.0, 2.0);
    bar.priceCurrentDayVwapUpperStdDevDiff = randomDouble(0.5, 2.0);
    bar.priceCurrentDayVwapLowerStdDevDiff = randomDouble(-2.0, -0.5);
    bar.pricePreviousDayVwapDiff = randomDouble(-3.0, 3.0);
    bar.priceWeeklyVwapDiff = randomDouble(-5.0, 5.0);
    
    // More price differences
    bar.priceBBandUpperDiff = randomDouble(1.0, 3.0);
    bar.priceBBandLowerDiff = randomDouble(-3.0, -1.0);
    bar.PriceBBandMiddleDiff = randomDouble(-1.0, 1.0);
    
    // TPO differences
    bar.isPriceInVA = randomDouble(0, 1) > 0.3;
    bar.priceCurrDayVAHDiff = randomDouble(0.5, 2.0);
    bar.priceCurrDayVALDiff = randomDouble(-2.0, -0.5);
    bar.pricePrevDayPOCDiff = randomDouble(-1.0, 1.0);
    bar.pricePrevDayVAHDiff = randomDouble(1.0, 3.0);
    bar.pricePrevDayVALDiff = randomDouble(-3.0, -1.0);
    
    // Support/Resistance differences
    bar.priceIBHighDiff = randomDouble(2.0, 4.0);
    bar.priceIBLowDiff = randomDouble(-4.0, -2.0);
    bar.pricePrevDayHighDiff = randomDouble(3.0, 6.0);
    bar.pricePrevDayLowDiff = randomDouble(-6.0, -3.0);
    bar.pricePrevDayCloseDiff = randomDouble(-2.0, 2.0);
    bar.priceWeekHighDiff = randomDouble(5.0, 10.0);
    bar.priceWeekLowDiff = randomDouble(-10.0, -5.0);
    
    // Technical levels
    bar.priceLastSwingHighDiff = randomDouble(2.0, 5.0);
    bar.priceLastSwingLowDiff = randomDouble(-5.0, -2.0);
    bar.priceLastHVNDiff = randomDouble(-3.0, 3.0);
    
    return bar;
}

// Create a demo day with the specified number of bars
Day createDemoDay(time_t dayStart, int numBars) {
    Day day;
    day.dayOfTheWeek = dayStart;
    
    // Add bars
    time_t barStart = dayStart + 34200; // Start at 9:30 AM
    for(int i = 0; i < numBars; i++) {
        day.bars.push_back(createDemoBar(barStart));
        barStart += 300; // Next 5-minute bar
    }
    
    // Set day-level calculations
    day.deltaZscore20bars = randomDouble(-2.0, 2.0);
    day.cumDelta5barSlope = randomDouble(-1.0, 1.0);
    day.priceCumDeltaDivergence5bar = randomDouble(-2.0, 2.0);
    day.priceCumDeltaDivergence10bar = randomDouble(-3.0, 3.0);
    day.interactionReversal20bar = randomDouble(-1.0, 1.0);
    
    // Technical indicators
    day.vwap = randomDouble(4000.0, 4010.0);
    day.vwapUpperStdDev = day.vwap + randomDouble(2.0, 4.0);
    day.vwapLowerStdDev = day.vwap - randomDouble(2.0, 4.0);
    day.bbMiddle = day.vwap;
    day.bbUpper = day.vwap + randomDouble(5.0, 8.0);
    day.bbLower = day.vwap - randomDouble(5.0, 8.0);
    day.BBandWidth = randomDouble(10.0, 20.0);
    day.rsi = randomDouble(30.0, 70.0);
    day.poc = day.vwap + randomDouble(-2.0, 2.0);
    day.vah = day.poc + randomDouble(3.0, 6.0);
    day.val = day.poc - randomDouble(3.0, 6.0);
    
    // Other day statistics
    day.ibHigh = day.vwap + randomDouble(4.0, 8.0);
    day.ibLow = day.vwap - randomDouble(4.0, 8.0);
    day.dayHigh = day.ibHigh + randomDouble(0.0, 4.0);
    day.dayLow = day.ibLow - randomDouble(0.0, 4.0);
    day.dayClose = randomDouble(day.dayLow, day.dayHigh);
    
    // Volume statistics
    day.totalVolume = randomDouble(100000, 200000);
    day.cumulativeDelta = randomDouble(-5000, 5000);
    day.lastSwingHigh = day.dayHigh - randomDouble(0.0, 2.0);
    day.lastSwingLow = day.dayLow + randomDouble(0.0, 2.0);
    day.lastHighVolumeNode = day.poc + randomDouble(-2.0, 2.0);
    
    return day;
}

// Create a demo week with the specified number of days
Week createDemoWeek(time_t weekStart, int numDays) {
    Week week;
    week.weekOfTheContract = weekStart;
    
    // Add days
    for(int i = 0; i < numDays; i++) {
        time_t dayStart = weekStart + (i * 86400); // 86400 seconds per day
        week.days.push_back(createDemoDay(dayStart, 10)); // 10 bars per day
    }
    
    // Set week-level calculations
    week.vwap = randomDouble(4000.0, 4010.0);
    week.poc = week.vwap + randomDouble(-2.0, 2.0);
    week.vah = week.poc + randomDouble(5.0, 10.0);
    week.val = week.poc - randomDouble(5.0, 10.0);
    week.weekHigh = week.vah + randomDouble(0.0, 5.0);
    week.weekLow = week.val - randomDouble(0.0, 5.0);
    
    return week;
}

void finalProcessing(Contract& contract) {
    // Get current time for the first week
    time_t currentTime = std::time(nullptr);
    
    // Create first week (5 days)
    std::cout << "Creating first week with 5 days..." << std::endl;
    Week firstWeek = createDemoWeek(currentTime, 5);
    contract.weeks.push_back(firstWeek);
    
    // Create second week (3 days), starting 7 days after first week
    std::cout << "Creating second week with 3 days..." << std::endl;
    time_t nextWeekStart = currentTime + (7 * 86400);
    Week secondWeek = createDemoWeek(nextWeekStart, 3);
    contract.weeks.push_back(secondWeek);
    
    std::cout << "Demo data generation complete." << std::endl;
}
```

### Description

This C++ file is responsible for generating realistic-looking, but entirely random, demo data for the trading bot. This is an essential tool for development and testing, as it allows you to work on the data structures and algorithms without needing to have the full data pipeline running.

### Logic

The code is structured in a hierarchical fashion, with helper functions to create each level of the data structure:

1.  **`randomDouble`**: A utility function to generate a random double-precision floating-point number within a specified range. This is used throughout the file to create varied and somewhat realistic data.

2.  **`createDemoFootprint`**: Creates a demo `Footprint` object. A footprint chart shows the volume traded at each price level within a bar. This function creates a few price levels with random bid and ask volumes and also determines if there is a buying or selling imbalance.

3.  **`createDemoBar`**: Creates a single `Bar` of data (e.g., a 5-minute bar). It populates all the fields in the `Bar` struct, including:
    *   OHLCV (Open, High, Low, Close, Volume) data.
    *   A `Footprint` chart created by the `createDemoFootprint` function.
    *   Various delta calculations (the difference between buying and selling volume).
    *   Placeholder signal data.
    *   A large number of features based on the price's difference from various technical levels (VWAP, Bollinger Bands, Value Area, etc.). These are the kinds of features that a machine learning model would use to make trading decisions.

4.  **`createDemoDay`**: Creates a `Day` object, which contains a vector of `Bar` objects. It also calculates and sets various day-level statistics and technical indicators.

5.  **`createDemoWeek`**: Creates a `Week` object, which contains a vector of `Day` objects, and sets week-level statistics.

6.  **`finalProcessing`**: This is the main function that is called from outside. It takes a `Contract` object and populates it with a couple of weeks of demo data.

### Explanation

The primary purpose of this file is to facilitate **rapid development and testing**. When you are building a complex system like a trading bot, you often have many different components that need to be developed in parallel. This demo data generator allows a developer working on, for example, a machine learning model or a backtesting engine to have a rich, structured dataset to work with long before the real data pipeline is complete. It ensures that the data structures are well-defined and that the downstream components can be built and tested independently.

## `training/main.cpp`

```cpp
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
```

### Description

This is the main entry point for the `training` executable. Its purpose is to generate a complete, structured, and feature-rich (though random) dataset and save it to a file. This dataset can then be used for training a machine learning model.

### Logic

1.  **Argument Parsing**: The program expects one command-line argument: the path to the output directory where the final data file will be saved.

2.  **Initialization**: A `Contract` object is created. This is the top-level data structure that will hold all the generated data.

3.  **Data Generation**: The `finalProcessing` function (which is defined in `finalProcessing.cpp`) is called. This function populates the `Contract` object with several weeks of realistic-looking demo data.

4.  **Output**: The program prints a summary of the data that was generated (number of weeks, days, and bars).

5.  **Serialization**: The `writeContractToJson` function is called to serialize the entire `Contract` data structure into a JSON file in the specified output directory.

6.  **Error Handling**: The entire process is wrapped in a `try...catch` block to handle any potential exceptions during data generation or file writing.

### Explanation

This program ties together the data structures defined in `dataStructure.h` and the demo data generation logic in `finalProcessing.cpp`. It serves as a convenient way to create a complete, self-contained dataset for testing and development. The output JSON file can be easily inspected or used as an input for other processes, such as the Python scripts for converting JSON to Parquet or for training a model.

## `training/dataStructure.h`

```cpp
#ifndef FOOTPRINT_DATA_STRUCTURE_H
#define FOOTPRINT_DATA_STRUCTURE_H

#include <map>
#include <vector>
#include <string>
#include <ctime>


// Volume information at a specific price level and time whitin the bar
// updation will be at each tick iteration
struct PriceLevel {
    double bidVolume;
    double askVolume;
    bool isBuyImbalance; // true for buy imbalance
    bool isSellImbalance; // true for sell imbalance
};


// footprint data for the bar
// updtion will be at each tick iteration
struct Footprint {
    std::map<double, PriceLevel> priceLevels;  // Time-based volume information
};



// bar data structure stores the data that can a single bar will hold within the whole cahrt data
// updation will occur at different frequency based on the calculations
struct Bar {
// calculated at each tick iteration
    Footprint footprint; // Footprint data of the bar
    int imbalanceCount; // Number of imbalances detected in the bar
    int delta; // Net volume delta (buy volume - sell volume)
    int barDeltaChange; // Change in delta from the previous bar
    int barHighDelta; // delta of the top two price levels in the bar
    int barLowDelta; // delta of the bottom two price levels in the bar
    double barPOC; // Point of Control price level in the bar
    
// bar's basic OHLCV data and will be calculated at each tick iteration
    time_t startTime;          // Start time of the bar
    time_t endTime;            // End time of the bar
    double open;         // Opening price
    double close;        // Closing price
    double high;         // Highest price
    double low;          // Lowest price
    double barTotalVolume;       // Total traded volume
    
// signal related data for the bar, will be updated once when signal is generated
// if signal will checked with many fixed set of rules(like minVolume, imbalance threshhold and etc) at each tick only if the signalStatus is false
// only one signal per candle
    int signal;       // Trading signal for the bar (2 for buy, 1 for sell, 0 for neutral)
    int signalID;     // Unique identifier for the signal, is the index of the tick when the signal was generated from the raw data
    bool signalStatus; // true if the signal is alresdy generated for the bar, false otherwise
    
// price-indicators difference value for the bar, will be calculated only if price changed
    double priceCurrentDayVwapDiff;      // Difference between last price and current day underdeveloping VWAP
    double priceCurrentDayVwapUpperStdDevDiff; // Difference between last price and Day VWAP + 1 Std Dev
    double priceCurrentDayVwapLowerStdDevDiff; // Difference between last price and Day VWAP - 1 Std Dev
    double pricePreviousDayVwapDiff;     // Difference between last price and previous day VWAP
    double priceWeeklyVwapDiff;          // Difference between last price and weekly VWAP
    double priceBBandUpperDiff;        // Difference between last price and upper Bollinger Band
    double priceBBandLowerDiff;        // Difference between last price and lower Bollinger Band
    double PriceBBandMiddleDiff;       // Difference between last price and middle Bollinger Band

// price-TPO difference value, will be calculated only if price changed
    bool isPriceInVA; // true if the last price is within the Value Area, false otherwise
    double priceCurrDayVAHDiff;    // Difference between last price and Value Area High of the current underdeveloping profile
    double priceCurrDayVALDiff;     // Difference between last price and Value Area Low of the current underdeveloping profile
    double pricePrevDayPOCDiff;          // Difference between last price and Point of Control of the previous day profile
    double pricePrevDayVAHDiff;   // Difference between last price and previous day Value Area High
    double pricePrevDayVALDiff;    // Difference between last price and previous day Value Area

// price S/R difference value, will be calculated only if price changed
    double priceIBHighDiff;       // Difference between last price and Initial Balance High
    double priceIBLowDiff;        // Difference between last price and Initial Balance Low
    double pricePrevDayHighDiff;   // Difference between last price and previous day high
    double pricePrevDayLowDiff;    // Difference between last price and previous day low
    double pricePrevDayCloseDiff;  // Difference between last price and previous day close
    double priceWeekHighDiff;      // Difference between last price and week high
    double priceWeekLowDiff;       // Difference between last price and week low

    double priceLastSwingHighDiff; // Difference between last price and last swing high
    double priceLastSwingLowDiff;  // Difference between last price and last swing low
    double priceLastHVNDiff;      // Difference between last price and last high volume node


};

struct Day {
    std::vector<Bar> bars;
    time_t dayOfTheWeek; 

// footprint related calculation
    double deltaZscore20bars; // Z-score of delta over the last 20 bars
    double cumDelta5barSlope; // Slope of cumulative delta over the last 5 bars
    double priceCumDeltaDivergence5bar; // Price and cumulative delta divergence measure
    double priceCumDeltaDivergence10bar; // Price and cumulative delta divergence measure
    double interactionReversal20bar; // Interaction reversal measure over the last 20 bars

// calculated on new bar creation
    double vwap; // VWAP for the day
    double vwapUpperStdDev; // Standard deviation of VWAP for the day
    double vwapLowerStdDev; // Standard deviation of VWAP for the day
    double bbMiddle; // Middle Bollinger Band for the day
    double bbUpper; // Upper Bollinger Band for the day
    double bbLower; // Lower Bollinger Band for the day
    double BBandWidth;                     // Width of the Bollinger Bands
    double rsi; // Relative Strength Index for the day
    double poc; // Point of Control for the day
    double vah; // Value Area High for the day
    double val; // Value Area Low for the day

//calculated once 
    double ibHigh; // Initial Balance High for the day , calculated for the first hour
    double ibLow; // Initial Balance Low for the day, calculated for the first hour

    double dayHigh; // Highest price of the day calculated at the end of the day
    double dayLow; // Lowest price of the day calculated at the end of the day
    double dayClose; // Closing price of the day calculated at the end of the day

    double totalVolume; // Total volume traded during the day
    double cumulativeDelta; // Cumulative delta for the day
    double lastSwingHigh; // Last swing high price of the day calculated at bar update
    double lastSwingLow; // Last swing low price of the day calculated at bar update
    double lastHighVolumeNode; // Last high volume node price of the day calculated at bar update


};

struct Week {
// calculated at the end of the week
    std::vector<Day> days;
    time_t weekOfTheContract; // Start date of the week
    double vwap; // VWAP for the week
    double poc; // Point of Control for the week
    double vah; // Value Area High for the week
    double val; // Value Area Low for the week
    double weekHigh; // Highest price of the week
    double weekLow; // Lowest price of the week
};


struct Contract{
    std::vector<Week> weeks;
    std::string contractName; // Name of the futures contract
    
};



#endif // FOOTPRINT_DATA_STRUCTURE_H
```

### Description

This header file is the backbone of the entire `training` application. It defines the data structures that are used to represent all the trading data in a hierarchical and organized manner. The comments in the file are particularly useful, as they explain when each piece of data is intended to be calculated (e.g., on every tick, on every new bar, at the end of the day).

### Logic

The data structures are organized in a nested hierarchy:

*   **`Contract`**: The top-level object. It contains a vector of `Week` objects and the name of the contract.
*   **`Week`**: Represents a single week of trading. It contains a vector of `Day` objects and various week-level calculations (VWAP, POC, etc.).
*   **`Day`**: Represents a single trading day. It contains a vector of `Bar` objects and many day-level calculations and technical indicators.
*   **`Bar`**: This is the most detailed structure. It represents a single time bar (e.g., 5 minutes) and contains a wealth of information:
    *   Basic OHLCV data.
    *   A `Footprint` object.
    *   Numerous calculated features, such as delta, imbalances, and the price's difference from various technical levels.
    *   Information about any trading signals generated within that bar.
*   **`Footprint`**: Contains a map of `PriceLevel` objects. This represents the volume that traded at each price within the bar.
*   **`PriceLevel`**: The most granular piece of information. It stores the bid and ask volume at a single price level and flags for buying or selling imbalances.

### Explanation

This file is a perfect example of how to structure complex data for a financial application. By breaking the data down into a logical hierarchy (Contract -> Week -> Day -> Bar -> Footprint -> PriceLevel), it becomes much easier to manage and reason about. The comments indicating the update frequency of each data member are also a great practice, as they help to understand the intended use of the data and how the system is expected to work.

This well-defined structure is what allows the `finalProcessing.cpp` file to generate such a rich and realistic dataset, and it provides a clear and organized input for any machine learning model that will be trained on this data.

## `training/database/database.h`

```cpp
#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <sqlite3.h>

struct TickData {
    int id;
    std::string Date;
    std::string Time;
    double Close;
    double AskVolume;
    double BidVolume;
};

class Database {
public:
    Database(const std::string& db_path);
    ~Database();
    std::vector<TickData> fetchData(const std::string& table_name, const std::string& date);

private:
    sqlite3* db;
};

#endif // DATABASE_H
```

### Description

This header file defines a simple C++ interface for interacting with a SQLite database. It provides a `TickData` structure to hold the data for a single tick and a `Database` class to manage the connection and data fetching.

### Logic

*   **`TickData` struct**: A simple structure to hold the raw tick data as it is read from the database.
*   **`Database` class**:
    *   **`Database(const std::string& db_path)`**: The constructor takes the path to the SQLite database file and opens a connection to it.
    *   **`~Database()`**: The destructor is responsible for closing the database connection.
    *   **`fetchData(const std::string& table_name, const std::string& date)`**: This is the main method of the class. It takes a table name and a date as input, and it is expected to return a vector of `TickData` structs containing all the ticks for that date from that table.

### Explanation

This file appears to be an early or alternative approach to database interaction. The `dataPipeline` executable uses the `SQLiteCpp` library, which is a more modern and feature-rich C++ wrapper for SQLite. This `Database` class, on the other hand, uses the raw C-style `sqlite3` API. While functional, using the raw API is more verbose and less safe (e.g., it doesn't use exceptions for error handling in the same way `SQLiteCpp` does).

It's possible that this was an initial implementation that was later replaced by the more robust solution in the `dataPipeline`. It's also possible that it's used for a different purpose, but given the context of the rest of the project, it's most likely a legacy or experimental component.

## `training/database/json_writer.cpp`

```cpp
#include "json_writer.h"
#include <iostream>
#include <fstream>
#include <sstream>

// Forward declarations for serialization functions
std::string priceLevelToJson(const PriceLevel& pl);
std::string footprintToJson(const Footprint& fp);
std::string barToJson(const Bar& bar);
std::string dayToJson(const Day& day);
std::string weekToJson(const Week& week);
std::string contractToJson(const Contract& contract);

std::string priceLevelToJson(const PriceLevel& pl) {
    std::stringstream ss;
    ss << "{";
    ss << "\"bidVolume\": " << pl.bidVolume << ",";
    ss << "\"askVolume\": " << pl.askVolume << ",";
    ss << "\"isBuyImbalance\": " << (pl.isBuyImbalance ? "true" : "false") << ",";
    ss << "\"isSellImbalance\": " << (pl.isSellImbalance ? "true" : "false");
    ss << "}";
    return ss.str();
}

std::string footprintToJson(const Footprint& fp) {
    std::stringstream ss;
    ss << "{";
    bool first = true;
    for (const auto& [price, level] : fp.priceLevels) {
        if (!first) {
            ss << ",";
        }
        ss << "\"" << price << "\": " << priceLevelToJson(level);
        first = false;
    }
    ss << "}";
    return ss.str();
}

std::string barToJson(const Bar& bar) {
    std::stringstream ss;
    ss << "{";
    ss << "\"startTime\": " << bar.startTime << ",";
    ss << "\"endTime\": " << bar.endTime << ",";
    ss << "\"open\": " << bar.open << ",";
    ss << "\"high\": " << bar.high << ",";
    ss << "\"low\": " << bar.low << ",";
    ss << "\"close\": " << bar.close << ",";
    ss << "\"barTotalVolume\": " << bar.barTotalVolume << ",";
    ss << "\"footprint\": " << footprintToJson(bar.footprint) << ",";
    ss << "\"imbalanceCount\": " << bar.imbalanceCount << ",";
    ss << "\"delta\": " << bar.delta << ",";
    ss << "\"barDeltaChange\": " << bar.barDeltaChange << ",";
    ss << "\"barHighDelta\": " << bar.barHighDelta << ",";
    ss << "\"barLowDelta\": " << bar.barLowDelta << ",";
    ss << "\"barPOC\": " << bar.barPOC << ",";
    ss << "\"signal\": " << bar.signal << ",";
    ss << "\"signalID\": " << bar.signalID << ",";
    ss << "\"signalStatus\": " << (bar.signalStatus ? "true" : "false") << ",";
    ss << "\"priceCurrentDayVwapDiff\": " << bar.priceCurrentDayVwapDiff << ",";
    ss << "\"priceCurrentDayVwapUpperStdDevDiff\": " << bar.priceCurrentDayVwapUpperStdDevDiff << ",";
    ss << "\"priceCurrentDayVwapLowerStdDevDiff\": " << bar.priceCurrentDayVwapLowerStdDevDiff << ",";
    ss << "\"pricePreviousDayVwapDiff\": " << bar.pricePreviousDayVwapDiff << ",";
    ss << "\"priceWeeklyVwapDiff\": " << bar.priceWeeklyVwapDiff << ",";
    ss << "\"priceBBandUpperDiff\": " << bar.priceBBandUpperDiff << ",";
    ss << "\"priceBBandLowerDiff\": " << bar.priceBBandLowerDiff << ",";
    ss << "\"PriceBBandMiddleDiff\": " << bar.PriceBBandMiddleDiff << ",";
    ss << "\"isPriceInVA\": " << (bar.isPriceInVA ? "true" : "false") << ",";
    ss << "\"priceCurrDayVAHDiff\": " << bar.priceCurrDayVAHDiff << ",";
    ss << "\"priceCurrDayVALDiff\": " << bar.priceCurrDayVALDiff << ",";
    ss << "\"pricePrevDayPOCDiff\": " << bar.pricePrevDayPOCDiff << ",";
    ss << "\"pricePrevDayVAHDiff\": " << bar.pricePrevDayVAHDiff << ",";
    ss << "\"pricePrevDayVALDiff\": " << bar.pricePrevDayVALDiff << ",";
    ss << "\"priceIBHighDiff\": " << bar.priceIBHighDiff << ",";
    ss << "\"priceIBLowDiff\": " << bar.priceIBLowDiff << ",";
    ss << "\"pricePrevDayHighDiff\": " << bar.pricePrevDayHighDiff << ",";
    ss << "\"pricePrevDayLowDiff\": " << bar.pricePrevDayLowDiff << ",";
    ss << "\"pricePrevDayCloseDiff\": " << bar.pricePrevDayCloseDiff << ",";
    ss << "\"priceWeekHighDiff\": " << bar.priceWeekHighDiff << ",";
    ss << "\"priceWeekLowDiff\": " << bar.priceWeekLowDiff << ",";
    ss << "\"priceLastSwingHighDiff\": " << bar.priceLastSwingHighDiff << ",";
    ss << "\"priceLastSwingLowDiff\": " << bar.priceLastSwingLowDiff << ",";
    ss << "\"priceLastHVNDiff\": " << bar.priceLastHVNDiff;
    ss << "}";
    return ss.str();
}

std::string dayToJson(const Day& day) {
    std::stringstream ss;
    ss << "{";
    ss << "\"dayOfTheWeek\": " << day.dayOfTheWeek << ",";
    ss << "\"bars\": [";
    bool first = true;
    for (const auto& bar : day.bars) {
        if (!first) {
            ss << ",";
        }
        ss << barToJson(bar);
        first = false;
    }
    ss << "]";
    ss << ",";
    ss << "\"deltaZscore20bars\": " << day.deltaZscore20bars << ",";
    ss << "\"cumDelta5barSlope\": " << day.cumDelta5barSlope << ",";
    ss << "\"priceCumDeltaDivergence5bar\": " << day.priceCumDeltaDivergence5bar << ",";
    ss << "\"priceCumDeltaDivergence10bar\": " << day.priceCumDeltaDivergence10bar << ",";
    ss << "\"interactionReversal20bar\": " << day.interactionReversal20bar << ",";
    ss << "\"vwap\": " << day.vwap << ",";
    ss << "\"vwapUpperStdDev\": " << day.vwapUpperStdDev << ",";
    ss << "\"vwapLowerStdDev\": " << day.vwapLowerStdDev << ",";
    ss << "\"bbMiddle\": " << day.bbMiddle << ",";
    ss << "\"bbUpper\": " << day.bbUpper << ",";
    ss << "\"bbLower\": " << day.bbLower << ",";
    ss << "\"BBandWidth\": " << day.BBandWidth << ",";
    ss << "\"rsi\": " << day.rsi << ",";
    ss << "\"poc\": " << day.poc << ",";
    ss << "\"vah\": " << day.vah << ",";
    ss << "\"val\": " << day.val << ",";
    ss << "\"ibHigh\": " << day.ibHigh << ",";
    ss << "\"ibLow\": " << day.ibLow << ",";
    ss << "\"dayHigh\": " << day.dayHigh << ",";
    ss << "\"dayLow\": " << day.dayLow << ",";
    ss << "\"dayClose\": " << day.dayClose << ",";
    ss << "\"totalVolume\": " << day.totalVolume << ",";
    ss << "\"cumulativeDelta\": " << day.cumulativeDelta << ",";
    ss << "\"lastSwingHigh\": " << day.lastSwingHigh << ",";
    ss << "\"lastSwingLow\": " << day.lastSwingLow << ",";
    ss << "\"lastHighVolumeNode\": " << day.lastHighVolumeNode;
    ss << "}";
    return ss.str();
}

std::string weekToJson(const Week& week) {
    std::stringstream ss;
    ss << "{";
    ss << "\"weekOfTheContract\": " << week.weekOfTheContract << ",";
    ss << "\"days\": [";
    bool first = true;
    for (const auto& day : week.days) {
        if (!first) {
            ss << ",";
        }
        ss << dayToJson(day);
        first = false;
    }
    ss << "]";
    ss << ",";
    ss << "\"vwap\": " << week.vwap << ",";
    ss << "\"poc\": " << week.poc << ",";
    ss << "\"vah\": " << week.vah << ",";
    ss << "\"val\": " << week.val << ",";
    ss << "\"weekHigh\": " << week.weekHigh << ",";
    ss << "\"weekLow\": " << week.weekLow;
    ss << "}";
    return ss.str();
}

std::string contractToJson(const Contract& contract) {
    std::stringstream ss;
    ss << "{";
    ss << "\"contractName\": \"" << contract.contractName << "\",";
    ss << "\"weeks\": [";
    bool first = true;
    for (const auto& week : contract.weeks) {
        if (!first) {
            ss << ",";
        }
        ss << weekToJson(week);
        first = false;
    }
    ss << "]";
    ss << "}";
    return ss.str();
}

void writeContractToJson(const Contract& contract, const std::string& output_dir) {
    std::string json_string = contractToJson(contract);
    std::ofstream outfile(output_dir + "/contract.json");
    outfile << json_string;
    outfile.close();
}
```

### Description

This C++ file is responsible for serializing the `Contract` data structure (and all its nested components) into a JSON string and writing it to a file. This is a crucial step for getting the data out of the C++ environment and into a format that can be easily used by other tools, especially the Python scripts in the project.

### Logic

The code consists of a set of recursive functions that mirror the hierarchy of the data structures:

*   **`priceLevelToJson`**: Converts a `PriceLevel` struct to a JSON object.
*   **`footprintToJson`**: Converts a `Footprint` struct to a JSON object. It iterates through the map of price levels and calls `priceLevelToJson` for each one.
*   **`barToJson`**: Converts a `Bar` struct to a JSON object. It calls `footprintToJson` for the nested `Footprint` object.
*   **`dayToJson`**: Converts a `Day` struct to a JSON object. It iterates through the vector of bars and calls `barToJson` for each one.
*   **`weekToJson`**: Converts a `Week` struct to a JSON object. It iterates through the vector of days and calls `dayToJson` for each one.
*   **`contractToJson`**: The top-level serialization function. It converts a `Contract` struct to a JSON object, calling `weekToJson` for each week.
*   **`writeContractToJson`**: This is the public-facing function. It takes a `Contract` object and an output directory, calls `contractToJson` to get the complete JSON string, and then writes that string to a file named `contract.json` in the specified directory.

### Explanation

This file demonstrates a manual approach to JSON serialization in C++. While it works, it's quite verbose and can be prone to errors (e.g., forgetting a comma or a quote). For more complex projects, it's common to use a dedicated JSON library (like `nlohmann/json` or `RapidJSON`) that can often automate this process, making the code cleaner and more robust.

However, for this project, the manual approach is understandable. The data structures are well-defined and the hierarchy is clear, so a manual implementation is feasible. The end result is a single JSON file that contains the entire dataset, which is a convenient format for the next step in the pipeline: converting the data to Parquet using Python.

## `training/database/json_writer.h`

```cpp
#ifndef JSON_WRITER_H
#define JSON_WRITER_H

#include "../dataStructure.h"
#include <string>

void writeContractToJson(const Contract& contract, const std::string& output_dir);

#endif // JSON_WRITER_H
```

### Description

This is the header file for the JSON writer. It declares the single function that is exposed to the rest of the application.

### Logic

*   **`#include "../dataStructure.h"`**: Includes the data structure definitions, which are necessary because the `writeContractToJson` function takes a `Contract` object as input.
*   **`void writeContractToJson(const Contract& contract, const std::string& output_dir)`**: This is the function declaration. It tells the compiler that there is a function named `writeContractToJson` that takes a constant reference to a `Contract` object and a constant reference to a string (the output directory) and returns nothing.

### Explanation

This is a straightforward header file that serves to decouple the implementation of the JSON writer from the code that uses it (in this case, `training/main.cpp`). By including this header, `main.cpp` knows about the `writeContractToJson` function without needing to know how it's implemented. This is a fundamental principle of good C++ design.

## `training/database/parquet_writer_simple.cpp`

```cpp
#include "parquet_writer.h"
#include <iostream>
#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/writer.h>
#include <arrow/builder.h>
#include <arrow/array.h>
#include <arrow/array/builder_nested.h>
#include <arrow/type.h>

// Simple schema with just essential fields
std::shared_ptr<arrow::Schema> getContractSchema() {
    auto barSchema = arrow::struct_({
        arrow::field("startTime", arrow::int64()),
        arrow::field("endTime", arrow::int64()),
        arrow::field("open", arrow::float64()),
        arrow::field("high", arrow::float64()),
        arrow::field("low", arrow::float64()),
        arrow::field("close", arrow::float64()),
        arrow::field("barTotalVolume", arrow::float64())
    });

    auto daySchema = arrow::struct_({
        arrow::field("dayOfTheWeek", arrow::int64()),
        arrow::field("bars", arrow::list(barSchema))
    });

    auto weekSchema = arrow::struct_({
        arrow::field("weekOfTheContract", arrow::int64()),
        arrow::field("days", arrow::list(daySchema))
    });

    return arrow::schema({
        arrow::field("contractName", arrow::utf8()),
        arrow::field("weeks", arrow::list(weekSchema))
    });
}

void writeContractToParquet(const Contract& contract, const std::string& output_dir) {
    std::cout << "Writing contract data to Parquet file..." << std::endl;
    
    try {
        // Create schema
        auto schema = getContractSchema();
        
        // Create builders
        arrow::MemoryPool* pool = arrow::default_memory_pool();
        arrow::StringBuilder contractNameBuilder(pool);
        
        // Create nested list builder for weeks
        auto weekStructType = std::static_pointer_cast<arrow::ListType>(schema->field(1)->type());
        std::unique_ptr<arrow::ListBuilder> weeksBuilder(
            new arrow::ListBuilder(pool, 
                std::make_shared<arrow::StructBuilder>(
                    weekStructType->value_type()->fields(),
                    pool
                )
            )
        );
        
        // Build contract name
        PARQUET_THROW_NOT_OK(contractNameBuilder.Append(contract.contractName));
        
        // Build weeks
        for (const auto& week : contract.weeks) {
            PARQUET_THROW_NOT_OK(weeksBuilder->Append());
            auto weekStructBuilder = static_cast<arrow::StructBuilder*>(weeksBuilder->value_builder());
            
            // Week of contract
            auto weekTimeBuilder = static_cast<arrow::Int64Builder*>(weekStructBuilder->field_builder(0));
            PARQUET_THROW_NOT_OK(weekTimeBuilder->Append(week.weekOfTheContract));
            
            // Days list
            auto daysBuilder = static_cast<arrow::ListBuilder*>(weekStructBuilder->field_builder(1));
            
            // Build days
            for (const auto& day : week.days) {
                PARQUET_THROW_NOT_OK(daysBuilder->Append());
                auto dayStructBuilder = static_cast<arrow::StructBuilder*>(daysBuilder->value_builder());
                
                // Day of week
                auto dayTimeBuilder = static_cast<arrow::Int64Builder*>(dayStructBuilder->field_builder(0));
                PARQUET_THROW_NOT_OK(dayTimeBuilder->Append(day.dayOfTheWeek));
                
                // Bars list
                auto barsBuilder = static_cast<arrow::ListBuilder*>(dayStructBuilder->field_builder(1));
                
                // Build bars
                for (const auto& bar : day.bars) {
                    PARQUET_THROW_NOT_OK(barsBuilder->Append());
                    auto barStructBuilder = static_cast<arrow::StructBuilder*>(barsBuilder->value_builder());
                    
                    // Bar fields
                    auto startTimeBuilder = static_cast<arrow::Int64Builder*>(barStructBuilder->field_builder(0));
                    auto endTimeBuilder = static_cast<arrow::Int64Builder*>(barStructBuilder->field_builder(1));
                    auto openBuilder = static_cast<arrow::DoubleBuilder*>(barStructBuilder->field_builder(2));
                    auto highBuilder = static_cast<arrow::DoubleBuilder*>(barStructBuilder->field_builder(3));
                    auto lowBuilder = static_cast<arrow::DoubleBuilder*>(barStructBuilder->field_builder(4));
                    auto closeBuilder = static_cast<arrow::DoubleBuilder*>(barStructBuilder->field_builder(5));
                    auto volumeBuilder = static_cast<arrow::DoubleBuilder*>(barStructBuilder->field_builder(6));
                    
                    PARQUET_THROW_NOT_OK(startTimeBuilder->Append(bar.startTime));
                    PARQUET_THROW_NOT_OK(endTimeBuilder->Append(bar.endTime));
                    PARQUET_THROW_NOT_OK(openBuilder->Append(bar.open));
                    PARQUET_THROW_NOT_OK(highBuilder->Append(bar.high));
                    PARQUET_THROW_NOT_OK(lowBuilder->Append(bar.low));
                    PARQUET_THROW_NOT_OK(closeBuilder->Append(bar.close));
                    PARQUET_THROW_NOT_OK(volumeBuilder->Append(bar.barTotalVolume));
                }
            }
        }
        
        // Finish arrays
        std::shared_ptr<arrow::Array> contractNameArray;
        std::shared_ptr<arrow::Array> weeksArray;
        
        PARQUET_THROW_NOT_OK(contractNameBuilder.Finish(&contractNameArray));
        PARQUET_THROW_NOT_OK(weeksBuilder->Finish(&weeksArray));
        
        // Create table
        auto table = arrow::Table::Make(schema, {contractNameArray, weeksArray});
        
        // Write to file
        std::string filename = output_dir + "/contract.parquet";
        std::cout << "Writing to file: " << filename << std::endl;
        
        std::shared_ptr<arrow::io::FileOutputStream> outfile;
        PARQUET_ASSIGN_OR_THROW(outfile, arrow::io::FileOutputStream::Open(filename));
        
        // Write with compression
        parquet::WriterProperties::Builder builder;
        builder.compression(parquet::Compression::SNAPPY);
        
        PARQUET_THROW_NOT_OK(
            parquet::arrow::WriteTable(
                *table,
                arrow::default_memory_pool(),
                outfile,
                1024 * 1024,  // chunk size
                builder.build()
            )
        );
        
        std::cout << "Successfully wrote contract data to parquet file" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error writing parquet file: " << e.what() << std::endl;
        throw;
    }
}
```

### Description

This C++ file provides an alternative way to write the `Contract` data to a file, this time using the Parquet format directly from C++. This is in contrast to the `json_writer.cpp`, which writes a JSON file that is later converted to Parquet by a Python script.

### Logic

1.  **`getContractSchema`**: This function defines the schema for the Parquet file. It uses the Apache Arrow library to define a nested structure that mirrors the `Contract` -> `Week` -> `Day` -> `Bar` hierarchy. Notably, this is a *simplified* schema that only includes the basic OHLCV data for each bar, not the full rich feature set from `dataStructure.h`.

2.  **`writeContractToParquet`**: This is the main function. It performs the following steps:
    *   Gets the schema from `getContractSchema`.
    *   Creates a set of nested Apache Arrow "builders". These are objects that allow you to efficiently build up the columns of your dataset in memory.
    *   It then iterates through the `Contract` data structure (weeks, days, and bars) and appends each piece of data to the appropriate builder.
    *   Once all the data has been added to the builders, it calls the `Finish()` method on each builder to create the final Arrow arrays.
    *   An Arrow `Table` is created from the schema and the arrays.
    *   Finally, it opens a file and uses `parquet::arrow::WriteTable` to write the Arrow `Table` to a Snappy-compressed Parquet file.

### Explanation

This file represents a more direct, and potentially more efficient, way of creating the Parquet file. By writing directly from C++, it avoids the intermediate step of creating a large JSON file. This can save both time and disk space.

However, there are a few interesting things to note:

*   **Simplified Schema**: This implementation only saves a small subset of the available data. This suggests that it might be a work in progress, or perhaps it was created for a specific purpose where only the basic bar data was needed.
*   **Complexity**: Writing nested structures to Parquet using the Arrow C++ builders is significantly more complex than the manual JSON serialization. It requires a deep understanding of the Arrow library and its builder classes. This is a trade-off: you get better performance and a more efficient file format, but the code is harder to write and maintain.
*   **Alternative Path**: This file, along with `parquet_writer.h` and `parquet.cpp`, seems to represent an alternative data pipeline that was either experimented with or is under development. The primary pipeline appears to be the C++ -> JSON -> Python -> Parquet route, but this shows that a direct C++ -> Parquet route was also being explored.

## `training/database/parquet_writer.h`

```cpp
#ifndef PARQUET_WRITER_H
#define PARQUET_WRITER_H

#include "../dataStructure.h"
#include <string>

void writeContractToParquet(const Contract& contract, const std::string& output_dir);

#endif // PARQUET_WRITER_H
```

### Description

This is the header file for the C++ Parquet writer. It's analogous to `json_writer.h` but for the Parquet writing functionality.

### Logic

*   **`#include "../dataStructure.h"`**: Includes the data structure definitions, as the `writeContractToParquet` function needs to know about the `Contract` struct.
*   **`void writeContractToParquet(const Contract& contract, const std::string& output_dir)`**: This declares the function that will be implemented in the `.cpp` files (`parquet_writer_simple.cpp` or `parquet.cpp`). It tells the rest of the program how to call the Parquet writing function.

### Explanation

This header file serves the same purpose as `json_writer.h`: it provides a clean interface for the Parquet writing functionality and decouples the implementation from the calling code. This allows you to switch out the implementation (e.g., between the simple version and a more complex one) without having to change the code that calls it.

## `training/database/parquet.cpp`

```cpp
// all the defination and functioon logics for the data feting and storing in the parquet files
```

### Description

This C++ file contains only a single comment.

### Logic

There is no executable code in this file.

### Explanation

This file is most likely a placeholder. The comment suggests that it was intended to contain the logic for fetching data and storing it in Parquet files. It's possible that the developer created this file with the intention of writing a more complex Parquet writer, but then implemented the `parquet_writer_simple.cpp` as a first version and has not yet gotten around to filling this one out. It could also be a remnant of a refactoring where the code was moved to another file.

## `training/database/sqlite.cpp`

```cpp
#include "database.h"
#include <iostream>
#include <vector>

Database::Database(const std::string& db_path) {
    if (sqlite3_open(db_path.c_str(), &db)) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    }
}

Database::~Database() {
    if (db) {
        sqlite3_close(db);
    }
}

static int callback(void* data, int argc, char** argv, char** azColName) {
    std::vector<TickData>* rows = static_cast<std::vector<TickData>*>(data);
    TickData row;
    for (int i = 0; i < argc; i++) {
        std::string colName(azColName[i]);
        if (colName == "id") {
            row.id = argv[i] ? std::stoi(argv[i]) : 0;
        } else if (colName == "Date") {
            row.Date = argv[i] ? argv[i] : "";
        } else if (colName == "Time") {
            row.Time = argv[i] ? argv[i] : "";
        } else if (colName == "Close") {
            row.Close = argv[i] ? std::stod(argv[i]) : 0.0;
        } else if (colName == "AskVolume") {
            row.AskVolume = argv[i] ? std::stod(argv[i]) : 0.0;
        } else if (colName == "BidVolume") {
            row.BidVolume = argv[i] ? std::stod(argv[i]) : 0.0;
        }
    }
    rows->push_back(row);
    return 0;
}

std::vector<TickData> Database::fetchData(const std::string& table_name, const std::string& date) {
    std::vector<TickData> data;
    if (!db) {
        return data;
    }

    std::string query = "SELECT id, Date, Time, Close, AskVolume, BidVolume FROM " + table_name + " WHERE Date = '" + date + "';";
    char* zErrMsg = 0;
    int rc = sqlite3_exec(db, query.c_str(), callback, &data, &zErrMsg);

    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }

    return data;
}

```

### Description

This C++ file provides the implementation for the `Database` class that was declared in `database.h`. It uses the C-style `sqlite3` API to interact with the SQLite database.

### Logic

*   **`Database::Database` (Constructor)**: Opens the SQLite database file. If there's an error, it prints a message to `std::cerr`.
*   **`Database::~Database` (Destructor)**: Closes the database connection if it's open.
*   **`callback` function**: This is a static C-style callback function that is required by the `sqlite3_exec` function. For each row returned by the SQL query, `sqlite3_exec` calls this function. The function then reconstructs the `TickData` struct from the raw C-style string arguments and pushes it into the vector of results.
*   **`Database::fetchData`**: This is the main data retrieval method. It constructs a SQL query string to select the required columns from the specified table and for the specified date. It then calls `sqlite3_exec`, passing the query, the callback function, and a pointer to the vector where the results should be stored. It also includes basic error handling.

### Explanation

This file confirms the suspicion from `database.h` that this is an alternative, lower-level way of interacting with the database compared to the `SQLiteCpp` library used in the main data pipeline. There are a few important things to note about this implementation:

*   **SQL Injection Vulnerability**: The way the SQL query is constructed by concatenating strings (`"... FROM " + table_name + " WHERE Date = '" + date + "';"`) makes it vulnerable to SQL injection attacks. While this may not be a major concern for an internal tool, it is a very bad practice. A better approach would be to use prepared statements and parameter binding, which is something that a library like `SQLiteCpp` makes much easier.
*   **C-style API**: The use of the `sqlite3_exec` function with a callback is a very C-like way of doing things. In modern C++, it's more common to use wrappers that provide a more object-oriented and type-safe interface.

Given these points, it is very likely that this code is either a legacy component that has been superseded by the `dataPipeline`'s more modern approach, or it was a quick implementation for a specific task where the developer didn't want to bring in the full `SQLiteCpp` dependency.

## `training/python/json_to_parquet.py`

```python
import json
import pandas as pd
import pyarrow as pa
import pyarrow.parquet as pq
import argparse

def json_to_parquet(json_file_path, parquet_file_path):
    """Reads a JSON file containing contract data, flattens it into a list of bars,
    converts it to a pandas DataFrame, and saves it as a Parquet file.
    """

    print(f"Reading JSON file: {json_file_path}")
    with open(json_file_path, 'r') as f:
        contract_data = json.load(f)

    print("Flattening JSON data...")
    bars_list = []
    for week in contract_data['weeks']:
        for day in week['days']:
            for bar in day['bars']:
                # You can add more fields from the day or week level here if needed
                bar['dayOfTheWeek'] = day['dayOfTheWeek']
                bar['weekOfTheContract'] = week['weekOfTheContract']
                bars_list.append(bar)

    print("Creating pandas DataFrame...")
    df = pd.json_normalize(bars_list, sep='_')

    print("Converting DataFrame to Arrow Table...")
    table = pa.Table.from_pandas(df)

    print(f"Writing Parquet file: {parquet_file_path}")
    pq.write_table(table, parquet_file_path)

    print("Conversion complete.")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Convert contract JSON to Parquet.')
    parser.add_argument('json_file', help='Input JSON file path.')
    parser.add_argument('parquet_file', help='Output Parquet file path.')
    args = parser.parse_args()

    json_to_parquet(args.json_file, args.parquet_file)
```

### Description

This Python script is a command-line tool for converting the JSON output from the C++ `training` application into the more efficient Parquet format. This is a key step in the data pipeline, as Parquet is much better suited for large-scale data analysis and machine learning than JSON.

### Logic

1.  **Argument Parsing**: The script uses the `argparse` module to accept two command-line arguments: the path to the input JSON file and the path for the output Parquet file.

2.  **`json_to_parquet` function**:
    *   **Read JSON**: It opens and reads the specified JSON file.
    *   **Flatten Data**: The core logic of the script is here. The nested JSON structure (Contract -> Weeks -> Days -> Bars) is not ideal for analysis. Most machine learning models expect a "flat" table-like structure. This part of the code iterates through the nested structure and creates a flat list of bars. For each bar, it also adds the `dayOfTheWeek` and `weekOfTheContract` information from the parent levels, so that this context is not lost.
    *   **Create DataFrame**: The `pd.json_normalize` function is used to convert the list of potentially nested JSON objects (the bars) into a flat pandas DataFrame. The `sep='_'` argument tells it to flatten any remaining nested structures (like the `footprint` data) by joining the keys with an underscore (e.g., `footprint_priceLevels_4000.0_bidVolume`).
    *   **Convert to Arrow Table**: The pandas DataFrame is converted into a PyArrow `Table`. PyArrow is a library that provides a standardized in-memory format for columnar data, and it's the foundation for Parquet in Python.
    *   **Write Parquet**: The `pq.write_table` function is used to write the Arrow `Table` to a Parquet file.

### Explanation

This script is a classic example of a data transformation task. It takes data in one format (nested JSON) and transforms it into another, more useful format (flat Parquet). This process is often called ETL (Extract, Transform, Load).

*   **Why not just use the C++ Parquet writer?** The project also contains a C++ Parquet writer (`parquet_writer_simple.cpp`). So why use this Python script? There are a few possible reasons:
    *   **Ease of Use**: As you can see from the code, flattening and transforming the data is significantly easier and more concise in Python with the pandas library than it would be in C++.
    *   **Flexibility**: If you want to change the way the data is flattened or add new features, it's much quicker to modify this Python script than to recompile the C++ code.
    *   **Full Feature Set**: The C++ Parquet writer was a simplified version that only wrote a subset of the data. This Python script, on the other hand, processes the full JSON output, ensuring that all the rich feature data is preserved in the final Parquet file.

This script is a critical link in the chain, preparing the data generated by the C++ application for the final step: analysis and model training.

## `training/python/readParquet.py`

```python
import pandas as pd

df = pd.read_parquet('contract.parquet')

print("\nColumn data types:")
print(df.dtypes)
```

### Description

This is a very simple Python script for inspecting the schema of a Parquet file.

### Logic

1.  **Import pandas**: Imports the pandas library.
2.  **Read Parquet**: Reads the `contract.parquet` file into a pandas DataFrame.
3.  **Print dtypes**: Prints the `dtypes` attribute of the DataFrame. This shows the name of each column and the data type that pandas has inferred for it.

### Explanation

This is another example of a small utility script used for debugging and verification. After running the `json_to_parquet.py` script, you can use this script to quickly check the schema of the resulting Parquet file. This is useful for ensuring that all the columns are present and have the correct data types before you proceed with more complex analysis or model training. It's a good practice to have these kinds of simple, focused scripts in your toolbox when working on a data-intensive project.
