import pandas as pd

path = '/media/sarjil/Vol2/Data/contract.parquet'
# path = '/media/sarjil/Vol2/Data/parquetData-esMINI/year=2024/month=01/day=03/data.parquet'

data = pd.read_parquet(path)

print(data.tail())














