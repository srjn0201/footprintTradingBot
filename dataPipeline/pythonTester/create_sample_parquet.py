
import pandas as pd
import numpy as np
import os

def create_sample_parquet(directory, num_files, num_rows):
    """
    Creates sample Parquet files with random data.

    Args:
        directory (str): The directory to save the Parquet files in.
        num_files (int): The number of Parquet files to create.
        num_rows (int): The number of rows per file.
    """
    if not os.path.exists(directory):
        os.makedirs(directory)

    for i in range(num_files):
        file_path = os.path.join(directory, f"sample_data_{i}.parquet")
        df = pd.DataFrame({
            'id': np.arange(i * num_rows, (i + 1) * num_rows),
            'value': np.random.rand(num_rows),
            'category': np.random.choice(['A', 'B', 'C'], num_rows)
        })
        df.to_parquet(file_path)
        print(f"Created {file_path}")

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Create sample Parquet files.")
    parser.add_argument("--dir", default="sample_parquet_data", help="Directory to save the Parquet files.")
    parser.add_argument("--num-files", type=int, default=2, help="Number of files to create.")
    parser.add_argument("--num-rows", type=int, default=10000, help="Number of rows per file.")
    args = parser.parse_args()

    create_sample_parquet(args.dir, args.num_files, args.num_rows)
