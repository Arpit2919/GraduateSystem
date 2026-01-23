#!/bin/bash

ROLL_NO="MT25017"
RESULTS="${ROLL_NO}_Part_C_CSV.csv"
PROGRAMS=("MT25017_Part_A_Program_A" "MT25017_Part_A_Program_B")
WORKERS=("cpu" "mem" "io")

echo "Program_Variant,Worker,CPU_Percent,Disk_Read_KB_s,Disk_Write_KB_s,Real_Time" > "$RESULTS"

for prog_name in "${PROGRAMS[@]}"; do
    prog="./$prog_name"

    if [ ! -x "$prog" ]; then
        echo "ERROR: $prog not found or not executable. Compile first!"
        continue
    fi

    for worker in "${WORKERS[@]}"; do
        echo "Executing $prog_name with $worker..."
        iostat -dk 1 5 > iostat_temp.txt &
        IOSTAT_PID=$!
        START=$(date +%s.%N)
        /usr/bin/time -f "%e %P" -o time_temp.txt \
            taskset -c 0 "$prog" "$worker" > /dev/null 2>&1
        EXIT_CODE=$?
        END=$(date +%s.%N)
        wait "$IOSTAT_PID" 2>/dev/null
        if [ $EXIT_CODE -eq 0 ] && [ -f time_temp.txt ]; then
            read REAL_TIME CPU_STR < time_temp.txt
            CPU_VAL=${CPU_STR%\%}
        else
            REAL_TIME=$(echo "scale=3; $END - $START" | bc)
            CPU_VAL="0.00"
        fi
        DISK_READ=$(awk '/(sd|nvme|vd|loop)/ {read+=$3; count++} \
                       END {if (count>0) print read/count; else print 0}' iostat_temp.txt)
        DISK_WRITE=$(awk '/(sd|nvme|vd|loop)/ {write+=$4; count++} \
                        END {if (count>0) print write/count; else print 0}' iostat_temp.txt)

        echo "$prog_name,$worker,$CPU_VAL,$DISK_READ,$DISK_WRITE,${REAL_TIME}s" >> "$RESULTS"

        rm -f iostat_temp.txt time_temp.txt
    done
done

echo "✅ Part C complete! Results: $RESULTS"
