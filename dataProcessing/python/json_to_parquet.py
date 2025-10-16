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
