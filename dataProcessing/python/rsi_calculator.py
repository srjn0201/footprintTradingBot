import math

def calculate_rsi_python_incremental(all_prices, period=14):
    # Initialize prevAvgGain and prevAvgLoss as NaN, similar to C++ struct
    prev_avg_gain = float('nan')
    prev_avg_loss = float('nan')
    current_rsi = float('nan')

    # We need at least 2 bars to calculate the first change
    if len(all_prices) < 2:
        return current_rsi, prev_avg_gain, prev_avg_loss

    # Simulate bar-by-bar processing
    # The loop starts from the second bar (index 1) because we need prices[i] and prices[i-1]
    for k in range(1, len(all_prices)): 
        # prices_up_to_k represents the 'PRICES' vector in the C++ code at each step
        prices_up_to_k = all_prices[:k+1]
        data_size = len(prices_up_to_k)

        # --- CASE 1: INITIALIZATION MODE ---
        # This condition mimics the C++ code: re-initialize if prev_avg_gain/loss are NaN
        # OR if data_size is less than 15 (which forces a re-initialization in C++ test context)
        if (math.isnan(prev_avg_gain) or math.isnan(prev_avg_loss) or data_size < 15):
            
            # C++ code's effectivePeriod logic
            effective_period = period
            if data_size < period + 1:
                effective_period = data_size - 1

            if effective_period < 1:
                prev_avg_gain = float('nan')
                prev_avg_loss = float('nan')
                current_rsi = float('nan')
                continue # Not enough data for even initial calculation

            # Calculate initial SMA for gains and losses over effective_period
            avg_gain_sum = 0.0
            avg_loss_sum = 0.0
            for i in range(1, effective_period + 1):
                change = prices_up_to_k[i] - prices_up_to_k[i-1]
                if change > 0:
                    avg_gain_sum += change
                else:
                    avg_loss_sum -= change
            
            prev_avg_gain = avg_gain_sum / effective_period
            prev_avg_loss = avg_loss_sum / effective_period

            # Apply smoothing for data beyond the effective_period, up to current data_size
            for i in range(effective_period + 1, data_size):
                change = prices_up_to_k[i] - prices_up_to_k[i-1]
                current_gain = change if change > 0 else 0.0
                current_loss = -change if change < 0 else 0.0
                
                prev_avg_gain = ((prev_avg_gain * (period - 1)) + current_gain) / period
                prev_avg_loss = ((prev_avg_loss * (period - 1)) + current_loss) / period

        # --- CASE 2: UPDATE MODE ---
        # If a valid previous state exists and data_size is sufficient, perform O(1) update.
        else: 
            # Get Newest Price Change (using the last two prices in prices_up_to_k)
            new_price = prices_up_to_k[-1]
            prev_price = prices_up_to_k[-2]
            change = new_price - prev_price

            current_gain = change if change > 0 else 0.0
            current_loss = -change if change < 0 else 0.0

            # Apply Wilder's Smoothing (The O(1) Update)
            prev_avg_gain = ((prev_avg_gain * (period - 1)) + current_gain) / period
            prev_avg_loss = ((prev_avg_loss * (period - 1)) + current_loss) / period

        # Final RSI Calculation for the current bar
        if prev_avg_loss == 0.0:
            current_rsi = 100.0
        elif prev_avg_gain == 0.0:
            current_rsi = 0.0
        else:
            rs = prev_avg_gain / prev_avg_loss
            current_rsi = 100.0 - (100.0 / (1.0 + rs))

    return current_rsi, prev_avg_gain, prev_avg_loss


# Test data from C++ test_rsi.cpp
initial_prices = [100.0] + [101, 102, 101, 102, 103, 104, 103, 102, 103, 104, 105, 106, 105, 106] # n=15
# Add more prices to reach n=50
additional_prices = [
    107.0, 108.0, 109.0, 110.0, 109.0, 108.0, 107.0, 106.0, 107.0, 108.0, # 10 bars (n=25)
    109.0, 110.0, 111.0, 112.0, 111.0, 110.0, 109.0, 108.0, 107.0, 106.0, # 10 bars (n=35)
    107.0, 108.0, 109.0, 110.0, 111.0, 112.0, 113.0, 114.0, 113.0, 112.0, # 10 bars (n=45)
    111.0, 110.0, 109.0, 108.0, 107.0 # 5 bars (n=50)
]

all_prices = initial_prices + additional_prices

# Calculate RSI for n=50
rsi_50, avg_gain_50, avg_loss_50 = calculate_rsi_python_incremental(all_prices, period=14)

print(f"RSI (n=50): {rsi_50}")
print(f"Avg Gain (n=50): {avg_gain_50}")
print(f"Avg Loss (n=50): {avg_loss_50}")
