import pandas as pd

df = pd.read_parquet('contract.parquet')

print("\nColumn data types:")
print(df.dtypes)