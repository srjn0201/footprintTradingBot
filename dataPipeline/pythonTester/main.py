import pandas as pd
import sqlite3
import argparse
from pathlib import Path

def parquet_to_sqlite(parquet_dir, db_path):
    """
    Reads all Parquet files from a directory and writes each one to a new
    table in a SQLite database.

    Args:
        parquet_dir (str): The path to the directory containing Parquet files.
        db_path (str): The path to the output SQLite database file.
    """
    # Find all Parquet files in the input directory
    parquet_files = list(Path(parquet_dir).glob("*.parquet"))
    if not parquet_files:
        print(f"No .parquet files found in {parquet_dir}")
        return

    print(f"Found {len(parquet_files)} Parquet files to process.")

    # Create a connection to the SQLite database
    try:
        with sqlite3.connect(db_path) as con:
            for file_path in sorted(parquet_files):
                table_name = file_path.stem  # Get filename without extension
                print(f"Processing {file_path.name} -> table '{table_name}'...", end="", flush=True)

                # Read the Parquet file into a pandas DataFrame
                df = pd.read_parquet(file_path)

                # Write the DataFrame to the SQLite table
                # if_exists='replace' will drop the table first if it exists
                # index=False prevents pandas from writing the DataFrame index as a column
                df.to_sql(table_name, con, if_exists='replace', index=False)
                print(" Done.")

        print(f"\nSuccessfully created SQLite database at {db_path}")

    except Exception as e:
        print(f"\nAn unexpected error occurred: {e}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Convert Parquet files to a SQLite database.")
    parser.add_argument("parquet_dir", help="The input directory containing Parquet files.")
    parser.add_argument("db_path", help="The path for the output SQLite database file.")

    args = parser.parse_args()

    parquet_to_sqlite(args.parquet_dir, args.db_path)
