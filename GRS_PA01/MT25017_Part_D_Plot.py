import pandas as pd
import matplotlib.pyplot as plt
import os

ROLL_NO = "MT25017_Plot"
CSV_FILE = "MT25017_Part_D_CSV.csv"

# Load data
if not os.path.exists(CSV_FILE):
    print(f"Error: {CSV_FILE} not found. Ensure your automation script has run.")
    exit()

df = pd.read_csv(CSV_FILE)

# 1. Data Cleaning
# Stripping 's' from time values and extracting N from variant names [cite: 35, 41]
df['Real_Time'] = df['Real_Time'].astype(str).str.replace('s', '', regex=False).astype(float)
df["N"] = df["Program_Variant"].str.extract(r'_(\d+)$').astype(int)

def plot_combined_metric(df, metric, ylabel):
    plt.figure(figsize=(12, 7))
    
    workers = ['cpu', 'mem', 'io']
    # Dashed lines for Program A (Processes), Solid for Program B (Threads) [cite: 14, 15]
    styles = {'Program_A': '--', 'Program_B': '-'} 
    colors = {'cpu': 'red', 'mem': 'blue', 'io': 'green'}

    for worker in workers:
        for prog_type in ['Program_A', 'Program_B']:
            subset = df[(df["Worker"] == worker) & (df["Program_Variant"].str.contains(prog_type))]
            subset = subset.sort_values("N")
            
            if not subset.empty:
                label = f"{prog_type} ({worker.upper()})"
                plt.plot(subset["N"], subset[metric], 
                         label=label, 
                         marker='o', 
                         linestyle=styles[prog_type], 
                         color=colors[worker],
                         linewidth=2)

    plt.xlabel("Number of Processes / Threads (N)")
    plt.ylabel(ylabel)
    plt.title(f"Combined Performance Analysis: {ylabel} vs N")
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.grid(True, linestyle='--', alpha=0.6)
    
    # Save directly to current directory using mandated naming convention [cite: 72]
    filename = f"{ROLL_NO}_Combined_{metric}.png"
    plt.savefig(filename, dpi=300, bbox_inches="tight")
    plt.close()
    print(f"Generated: {filename}")

# Generate the three main combined graphs for Part D [cite: 58, 59]
metrics = [
    ("CPU_Percent", "CPU Utilization (%)"),
    ("Disk_Write_KB_s", "Disk Write Throughput (KB/s)"),
    ("Real_Time", "Total Execution Time (s)")
]

for metric, ylabel in metrics:
    plot_combined_metric(df, metric, ylabel)

print("\n✅ All 3 combined plots are now in your current directory.")