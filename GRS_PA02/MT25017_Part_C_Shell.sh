#!/bin/bash
# MT25017 - Network I/O Performance Evaluation (FINAL + BYTES LOGGED)

set -euo pipefail

SERVER_NS="server_ns"
CLIENT_NS="client_ns"
SERVER_IP="10.0.0.1"
PORT=8080
DURATION=10

VERSIONS=("A1" "A2" "A3")
SIZES=(256 512 1024 4096)
THREADS=(1 2 4 8)

CSV="MT25017_Part_C_CSV.csv"

# =====================================================
# CSV HEADER (UPDATED + BYTES ADDED)
# =====================================================
echo "version,msg_size,threads_count,total_bytes,throughput_gbps,latency_us,cpu_cycles,cache_misses" > "$CSV"


command -v bc >/dev/null || { echo "Install bc"; exit 1; }
command -v perf >/dev/null || { echo "Install linux-perf"; exit 1; }


cleanup_processes() {
    sudo pkill -INT -f MT25017_Part.*_Server >/dev/null 2>&1 || true
    sudo pkill -INT -f MT25017_Part.*_Client >/dev/null 2>&1 || true
    sleep 1
}
trap cleanup_processes EXIT


check_ns() {
    sudo ip netns list | grep -q "$SERVER_NS" || { echo "Missing $SERVER_NS"; exit 1; }
    sudo ip netns list | grep -q "$CLIENT_NS" || { echo "Missing $CLIENT_NS"; exit 1; }
}


compile_all() {
    echo "Compiling..."

    gcc -O2 -pthread -o MT25017_PartA1_Server MT25017_PartA1_Server.c
    gcc -O2 -pthread -o MT25017_PartA1_Client MT25017_PartA1_Client.c

    gcc -O2 -pthread -o MT25017_PartA2_Server MT25017_PartA2_Server.c
    gcc -O2 -pthread -o MT25017_PartA2_Client MT25017_PartA2_Client.c

    gcc -O2 -pthread -o MT25017_PartA3_Server MT25017_PartA3_Server.c
    gcc -O2 -pthread -o MT25017_PartA3_Client MT25017_PartA3_Client.c
}


run_test() {

    VERSION=$1
    SIZE=$2
    THREADS_COUNT=$3

    SERVER="./MT25017_Part${VERSION}_Server"
    CLIENT="./MT25017_Part${VERSION}_Client"

    echo "▶ $VERSION | size=$SIZE | threads=$THREADS_COUNT"

    PERF_FILE=$(mktemp)

    cleanup_processes

    sudo ip netns exec $SERVER_NS perf stat \
        -e cycles,cache-misses \
        "$SERVER" $PORT $SIZE \
        > "$PERF_FILE" 2>&1 &

    SERVER_PID=$!
    sleep 2

    for ((i=0;i<THREADS_COUNT;i++)); do
        sudo ip netns exec $CLIENT_NS "$CLIENT" $SERVER_IP $PORT $SIZE $DURATION \
            > /dev/null 2>&1 &
    done

    sleep $DURATION

    sudo kill -INT $SERVER_PID 2>/dev/null || true
    wait $SERVER_PID 2>/dev/null || true


    # ======================
    # PARSE
    # ======================

    TOTAL_BYTES=$(grep -o "Total bytes sent: [0-9]*" "$PERF_FILE" | awk '{print $4}' || echo 0)

    if [ "$TOTAL_BYTES" -gt 0 ]; then
        THROUGHPUT=$(echo "scale=6; ($TOTAL_BYTES*8)/($DURATION*1000000000)" | bc)
        MSGS=$((TOTAL_BYTES / SIZE))
        LATENCY=$(echo "scale=2; ($DURATION*1000000)/$MSGS" | bc)
    else
        THROUGHPUT=0
        LATENCY=0
    fi

    CPU_CYCLES=$(grep cycles "$PERF_FILE" | awk '{gsub(",",""); print $1}')
    CACHE_MISSES=$(grep cache-misses "$PERF_FILE" | awk '{gsub(",",""); print $1}')

    CPU_CYCLES=${CPU_CYCLES:-0}
    CACHE_MISSES=${CACHE_MISSES:-0}


    # ======================
    # WRITE CSV (BYTES INCLUDED)
    # ======================
    echo "$VERSION,$SIZE,$THREADS_COUNT,$TOTAL_BYTES,$THROUGHPUT,$LATENCY,$CPU_CYCLES,$CACHE_MISSES" >> "$CSV"

    echo "   ✓ ${THROUGHPUT} Gbps | ${LATENCY} µs"

    rm -f "$PERF_FILE"
}


check_ns
compile_all

for V in "${VERSIONS[@]}"; do
    for S in "${SIZES[@]}"; do
        for T in "${THREADS[@]}"; do
            run_test "$V" "$S" "$T"
        done
    done
done

echo "Done. Results saved → $CSV"
