import pandas as pd
import sqlite3
import pyarrow.parquet as pq
import pyarrow as pa
from datetime import timedelta

def transfer_last_month_data(parquet_path, db_path, table_name):
    """
    Reads a Parquet file, finds the last 30 days of data present in the file,
    and writes it to a SQLite database.

    Args:
        parquet_path (str): The path to the Parquet file.
        db_path (str): The path to the output SQLite database file.
        table_name (str): The name of the table to create in the database.
    """
    try:
        parquet_file = pq.ParquetFile(parquet_path)
        schema = parquet_file.schema.to_arrow_schema()

        timestamp_column = None
        for field in schema:
            if pa.types.is_timestamp(field.type):
                timestamp_column = field.name
                break

        if timestamp_column is None:
            print("No timestamp column found in the Parquet file.")
            return

        print(f"Found timestamp column: {timestamp_column}")

        # Read the last row group to find the latest timestamp
        print("Reading the last part of the file to find the latest timestamp...")
        num_row_groups = parquet_file.num_row_groups
        if num_row_groups == 0:
            print("No data found in the Parquet file.")
            return
        
        last_row_group = parquet_file.read_row_group(num_row_groups - 1)
        latest_timestamp = last_row_group.column(timestamp_column)[-1].as_py()

        print(f"Latest timestamp found: {latest_timestamp}")

        # Calculate the start date for the last 30 days of data
        start_date = latest_timestamp - timedelta(days=30)
        print(f"Filtering data from {start_date} to {latest_timestamp}")

        # Filter and write the data
        with sqlite3.connect(db_path) as con:
            first_chunk = True
            for batch in parquet_file.iter_batches(batch_size=100000):
                df = batch.to_pandas()
                df[timestamp_column] = pd.to_datetime(df[timestamp_column])
                
                # Filter the DataFrame
                filtered_df = df[(df[timestamp_column] >= start_date) & (df[timestamp_column] <= latest_timestamp)]
                
                if not filtered_df.empty:
                    if_exists_mode = 'replace' if first_chunk else 'append'
                    filtered_df.to_sql(table_name, con, if_exists=if_exists_mode, index=False)
                    first_chunk = False
                    print(f"Processed a chunk of data. Found {len(filtered_df)} rows in the date range.")

        print(f"\nSuccessfully transferred last 30 days of data to {db_path}")

    except Exception as e:
        print(f"\nAn unexpected error occurred: {e}")

if __name__ == "__main__":
    parquet_path = "/media/sarjil/Vol2/Data/dataFromFootprint/parquet raw data/ESH24_tick.parquet"
    db_path = "/home/sarjil/sarjil_u/C++/footprintTradingBot/dataPipeline/pythonTester/converted_database.db"
    table_name = "ESH24_tick"
    
    transfer_last_month_data(parquet_path, db_path, table_name)