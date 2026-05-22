import pandas as pd
import matplotlib.pyplot as plt
import os

# Load our hardware dataset
df = pd.read_csv('results/hardware_dataset.csv')

# Group by test type and calculate mean values
summary = df.groupby('Type').mean()

# Set up an elegant, academic plotting style
plt.style.use('seaborn-v0_8-whitegrid')
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))

# Plot 1: Execution Time comparison
colors = ['#4A90E2', '#D0021B'] # Aligned vs Contended branding
bars1 = ax1.bar(summary.index, summary['Seconds'], color=['#4A90E2', '#D0021B'], width=0.4, edgecolor='black')
ax1.set_title('Mean Execution Time (Lower is Better)', fontsize=12, fontweight='bold')
ax1.set_ylabel('Seconds', fontsize=11)
ax1.set_ylim(0, max(summary['Seconds']) * 1.2)
for bar in bars1:
    yval = bar.get_height()
    ax1.text(bar.get_x() + bar.get_width()/2.0, yval + 0.1, f"{yval:.3f}s", ha='center', va='bottom', fontweight='bold')

# Plot 2: L1 Data Cache Misses comparison (Log Scale because the drop is massive)
bars2 = ax2.bar(summary.index, summary['L1_Misses'], color=['#4A90E2', '#D0021B'], width=0.4, edgecolor='black')
ax2.set_yscale('log') # Logarithmic scale since aligned misses are tiny compared to contended
ax2.set_title('Mean L1 D-Cache Load Misses (Log Scale)', fontsize=12, fontweight='bold')
ax2.set_ylabel('Misses (Logarithmic)', fontsize=11)
for bar in bars2:
    yval = bar.get_height()
    ax2.text(bar.get_x() + bar.get_width()/2.0, yval * 1.5, f"{int(yval):,}", ha='center', va='bottom', fontweight='bold')

plt.suptitle('Empirical Impact of False Sharing on Intel Haswell (i5-4200U)', fontsize=14, fontweight='bold', y=1.02)
plt.tight_layout()

# Save the visualization to our results folder
os.makedirs('results', exist_ok=True)
plt.savefig('results/cache_performance_cliff.png', dpi=300, bbox_inches='tight')
print("Graph generated and saved successfully to results/cache_performance_cliff.png")
