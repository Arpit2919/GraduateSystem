import matplotlib.pyplot as plt

# =====================================================
# Hardcoded data extracted from YOUR CSV
# =====================================================

msg_sizes = [256, 512, 1024, 4096]
threads   = [1, 2, 4, 8]

# -----------------------------------------------------
# Throughput (Gbps)
# -----------------------------------------------------
throughput = {
    "A1": [2.499237, 4.612535, 7.926711, 27.115556],
    "A2": [2.148378, 3.191497, 8.994417, 24.228636],
    "A3": [0.037583, 0.684215, 1.424288, 9.834262]
}

# -----------------------------------------------------
# Latency vs THREAD COUNT (256B case from CSV)
# -----------------------------------------------------
latency = {
    "A1": [0.81, 0.78, 1.32, 1.17],
    "A2": [0.95, 0.84, 0.98, 1.42],
    "A3": [54.49, 5.11, 5.13, 5.81]
}

# -----------------------------------------------------
# Cache Misses (threads=1 rows)
# -----------------------------------------------------
cache_misses = {
    "A1": [91861625, 131978044, 222724274, 686333400],
    "A2": [80078424, 116011139, 244352048, 483578614],
    "A3": [7003543, 15592609, 19395849, 9759669]
}

# -----------------------------------------------------
# CPU cycles per byte
# cycles_per_byte = cpu_cycles / total_bytes
# -----------------------------------------------------
cycles_per_byte = {
    "A1": [12.55, 6.25, 3.43, 1.39],
    "A2": [17.92, 9.87, 5.33, 8.47],
    "A3": [1488.15, 73.61, 37.32, 5.02]
}

# =====================================================
# Plot helper
# =====================================================
def plot_graph(x, data, xlabel, ylabel, filename, logx=True):

    plt.figure(figsize=(8,5))

    for label, y in data.items():
        plt.plot(x, y, marker='o', linewidth=2, label=label)

    if logx:
        plt.xscale("log", base=2)

    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.grid(True)
    plt.legend()

    plt.tight_layout()
    plt.savefig(filename)
    plt.close()


# =====================================================
# Generate 4 required graphs
# =====================================================

plot_graph(msg_sizes, throughput,
           "Message Size (bytes)", "Throughput (Gbps)",
           "throughput.png")

plot_graph(threads, latency,
           "Thread Count", "Latency (µs)",
           "latency.png", logx=False)

plot_graph(msg_sizes, cache_misses,
           "Message Size (bytes)", "Cache Misses",
           "cache_misses.png")

plot_graph(msg_sizes, cycles_per_byte,
           "Message Size (bytes)", "CPU Cycles per Byte",
           "cycles_per_byte.png")


print("Generated:")
print(" throughput.png")
print(" latency.png")
print(" cache_misses.png")
print(" cycles_per_byte.png")
