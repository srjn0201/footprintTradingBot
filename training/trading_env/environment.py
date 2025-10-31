# rl_trading_bot/trading_env/stock_trading_env.py

import gymnasium as gym
import numpy as np
import pandas as pd

class StockTradingEnv(gym.Env):
    """A stock trading environment for reinforcement learning"""
    metadata = {'render_modes': ['human']}

    def __init__(self, df):
        super(StockTradingEnv, self).__init__()

        self.df = df
        self.reward_range = (-np.inf, np.inf)
        self.current_step = 0
        self.initial_balance = 10000
        self.balance = self.initial_balance
        self.shares_held = 0
        self.net_worth = self.initial_balance

        # Action space: A continuous value between -1 (sell) and 1 (buy)
        self.action_space = gym.spaces.Box(
            low=-1, high=1, shape=(1,), dtype=np.float32
        )

        # Observation space:
        self.observation_space = gym.spaces.Box(
            low=0, high=np.inf, shape=(3,), dtype=np.float32
        )

    def _next_observation(self):
        # Get the stock data at the current step
        current_price = self.df.loc[self.current_step, 'Close']
        
        obs = np.array([
            self.balance,
            self.shares_held,
            current_price,
        ], dtype=np.float32)
        return obs

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        
        # Reset the state of the environment to an initial state
        self.balance = self.initial_balance
        self.net_worth = self.initial_balance
        self.shares_held = 0
        self.current_step = 0
        
        observation = self._next_observation()
        info = {}
        return observation, info

    def step(self, action):
        # Execute one time step within the environment
        self._take_action(action)

        self.current_step += 1

        # Calculate reward
        reward = self.net_worth - self.initial_balance
        
        # Check if the episode is done
        done = self.current_step >= len(self.df) - 1

        # Get next observation
        observation = self._next_observation()
        
        info = {}
        
        return observation, reward, done, done, info

    def _take_action(self, action):
        current_price = self.df.loc[self.current_step, 'Close']
        
        # FIX: Extract the scalar value from the action array
        action_value = action[0]

        if action_value > 0:  # Buy
            # Buy a proportion of available balance
            buy_amount = self.balance * action_value
            shares_to_buy = buy_amount / current_price
            self.shares_held += shares_to_buy
            self.balance -= buy_amount

        elif action_value < 0:  # Sell
            # Sell a proportion of held shares
            shares_to_sell = self.shares_held * abs(action_value)
            self.shares_held -= shares_to_sell
            self.balance += shares_to_sell * current_price

        # Update net worth
        self.net_worth = self.balance + self.shares_held * current_price

    def render(self, mode='human', close=False):
        # Render the environment to the screen
        profit = self.net_worth - self.initial_balance
        print(f'Step: {self.current_step}')
        print(f'Balance: {self.balance:.2f}')
        print(f'Shares held: {self.shares_held:.2f}')
        print(f'Net Worth: {self.net_worth:.2f}')
        print(f'Profit: {profit:.2f}')