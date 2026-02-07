# MT25017

## Project Structure
Part A1: Two-copy implementation (baseline)
Part A2: One-copy implementation  
Part A3: Zero-copy implementation
Part B: Performance measurements
Part C: Automation script
Part D: Plotting scripts
Part E: Analysis



Overview:
This assignment evaluates the performance of three network I/O implementations:

Version	Technique:
A1	Two-copy (send/recv)
A2	One-copy (single buffer optimization)
A3	Zero-copy (mmap/splice based)

We measure:
Throughput (Gbps)
Latency (µs)
CPU cycles
Cache misses
Total bytes transferred
Linux network namespaces are used to isolate client/server and perf is used for hardware performance counters.


PROJECT STRUCTURE:
.
├── MT25017_PartA1_Server.c
├── MT25017_PartA1_Client.c
├── MT25017_PartA2_Server.c
├── MT25017_PartA2_Client.c
├── MT25017_PartA3_Server.c
├── MT25017_PartA3_Client.c
│
├── MT25017_Part_C_Shell.sh     # shell benchmark script
├── MT2017_Part_D_Plot.py       # matplotlib plotting script
├── MT25017_Part_C_CSV.csv      # results generated automatically
├── README.md


REQUIREMENTS:
sudo apt update
sudo apt install gcc make bc linux-perf python3 python3-matplotlib python3-pandas

SET UP NETWORK NAMESPACES:
sudo ip netns add server_ns
sudo ip netns add client_ns

sudo ip link add veth0 type veth peer name veth1

sudo ip link set veth0 netns server_ns
sudo ip link set veth1 netns client_ns

sudo ip netns exec server_ns ip addr add 10.0.0.1/24 dev veth0
sudo ip netns exec client_ns ip addr add 10.0.0.2/24 dev veth1

sudo ip netns exec server_ns ip link set veth0 up
sudo ip netns exec client_ns ip link set veth1 up
sudo ip netns exec server_ns ip link set lo up
sudo ip netns exec client_ns ip link set lo up


Step 2 — Run experiments
Give permission:
chmod +x MT25017_Part_C_Shell.sh
./MT25017_Part_C_Shell.sh

Step 3 — Output file
MT25017_Part_C_CSV.csv

Step 4 — Generate plots
python3 MT25017_Part_D_Plot.py

Generated graphs:
throughput.png,latency.png,cache_misses.png,cycles_per_byte.png

Metrics Explained:
Throughput:(bytes * 8) / time
Latency:total_time / number_of_messages
CPU Cycles:Measured using perf on server side.
Cache Misses:Measured using perf (LLC misses).
Cycles per byte:cpu_cycles / total_bytes

