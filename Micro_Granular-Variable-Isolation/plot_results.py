import pandas as pd
import matplotlib.pyplot as plt

# 1. Load the raw empirical data
try:
    df = pd.read_csv('cache_cliff_results.csv')
except FileNotFoundError:
    print("Error: cache_cliff_results.csv not found. Please run the bash script first.")
    exit(1)

# 2. Initialize a dual-axis plot to show both Time and Cache Misses simultaneously
fig, ax1 = plt.subplots(figsize=(10, 6))

# Plot Execution Duration (MS) on the left Y-axis
color = '#d62728' # Scientific Red
ax1.set_xlabel('Variable Isolation Padding (Bytes)', fontsize=12, fontweight='bold')
ax1.set_ylabel('Execution Time (ms)', color=color, fontsize=12, fontweight='bold')
line1 = ax1.plot(df['Padding_Bytes'], df['Duration_MS'], color=color, marker='o', linewidth=2, label='Execution Time (ms)')
ax1.tick_params(axis='y', labelcolor=color)
ax1.grid(True, linestyle='--', alpha=0.6)

# Create a second Y-axis sharing the same X-axis for L1 Cache Misses
ax2 = ax1.twinx()  
color = '#1f77b4' # Scientific Blue
ax2.set_ylabel('L1 Data Cache Load Misses', color=color, fontsize=12, fontweight='bold')
line2 = ax2.plot(df['Padding_Bytes'], df['L1_Dcache_Misses'], color=color, marker='s', linestyle='--', linewidth=2, label='L1 Cache Misses')
ax2.tick_params(axis='y', labelcolor=color)

# Add an explicit indicator line at the 64-byte hardware boundary
plt.axvline(x=64, color='black', linestyle=':', linewidth=2, label='Processor Cache Line Boundary (64B)')

# Combine legends from both axes
lines = line1 + line2
labels = [l.get_label() for l in lines]
ax1.legend(lines, labels, loc='upper right')

plt.title('Empirical Analysis of False Sharing: The Microarchitectural Cliff', fontsize=14, fontweight='bold', pad=15)
fig.tight_layout()

# Save the chart as a high-resolution figure ready for a research paper
plt.savefig('cache_performance_cliff.png', dpi=300)
print("Success! Chart saved as 'cache_performance_cliff.png'")
plt.show()

