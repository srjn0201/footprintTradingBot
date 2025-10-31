# rl_trading_bot/train.py

import pandas as pd
from stable_baselines3 import SAC
from trading_env.environment import StockTradingEnv
import sqlite3


path = "/home/sarjil/sarjil_u/C++/footprintTradingBot/testData/tdata_1m.db"

# Establish a connection to the SQLite database
conn = sqlite3.connect(path)

# Fetch all data from the 'ITC' table
df = pd.read_sql_query("SELECT * FROM ITC", conn)

# Close the database connection
conn.close()


# Create the trading environment
env = StockTradingEnv(df)

# Instantiate the SAC agent
# 'MlpPolicy' is a standard feed-forward neural network policy.
model = SAC('MlpPolicy', env, verbose=1, tensorboard_log="./logs/")

# Train the agent
# total_timesteps is the number of steps the agent will take in the environment to learn.
model.learn(total_timesteps=20000, progress_bar=True)

# Save the trained model
model.save("models/sac_stock_trader")

print("Training complete. Model saved.")

# --- To evaluate the trained agent (optional) ---

# del model # remove to demonstrate saving and loading
# model = SAC.load("models/sac_stock_trader")

# obs, info = env.reset()
# for i in range(len(df) - 1):
#     action, _states = model.predict(obs, deterministic=True)
#     obs, rewards, terminated, truncated, info = env.step(action)
#     env.render()
#     if terminated or truncated:
#         print("Evaluation finished.")
#         break