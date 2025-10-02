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
        # For SQLite, the URI format is sqlite:/// followed by the absolute path.
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