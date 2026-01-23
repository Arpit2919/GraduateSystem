#!/bin/bash

ROLL_NO="MT25017"
RESULTS="${ROLL_NO}_Part_D_CSV.csv"

echo "Program_Variant,Worker,CPU_Percent,Disk_Read_KB_s,Disk_Write_KB_s,Real_Time" > "$RESULTS"

for prog in MT25017_Part_A_Program_A MT25017_Part_A_Program_B; do
    if [[ $prog == *_A ]]; then 
        Ns=(2 3 4 5)
    else
        Ns=(2 3 4 5 6 7 8)
    fi
    for n in "${Ns[@]}"; do
        for w in cpu mem io; do
            prog_path="./$prog"
            if [ ! -x "$prog_path" ]; then
                echo "ERROR: $prog_path missing!"
                continue
            fi
            
            echo "Running $prog $n $w..."
            
            # Background iostat
            iostat -dk 1 8 > "iostat_${prog}_${n}_${w}.txt" &
            IOSTAT_PID=$!
            
            START=$(date +%s.%N)
            if [[ $prog == *_A ]]; then
                # Fork: single core
                /usr/bin/time -f "%e %P" -o "time_${prog}_${n}_${w}.txt" \
                    timeout 15s taskset -c 0 "$prog_path" "$n" "$w" > /dev/null 2>&1
            else
                # Pthread: spread cores (safer)
                /usr/bin/time -f "%e %P" -o "time_${prog}_${n}_${w}.txt" \
                    timeout 15s taskset -c 0-3 "$prog_path" "$n" "$w" > /dev/null 2>&1
            fi
            EXIT_CODE=$?
            END=$(date +%s.%N)
            
            wait "$IOSTAT_PID" 2>/dev/null
            
            if [ $EXIT_CODE -eq 0 ] && [ -f "time_${prog}_${n}_${w}.txt" ]; then
                read REAL_TIME CPU_STR < "time_${prog}_${n}_${w}.txt"
                CPU_VAL=${CPU_STR%\%}
            else
                REAL_TIME=$(echo "scale=3; $END - $START" | bc)
                CPU_VAL="0.00"
            fi
            
            DISK_READ=$(awk '/(sd|nvme|vd|loop)/ {read+=$3; count++} \
                           END {if (count>0) print read/count; else print 0}' "iostat_${prog}_${n}_${w}.txt")
            DISK_WRITE=$(awk '/(sd|nvme|vd|loop)/ {write+=$4; count++} \
                            END {if (count>0) print write/count; else print 0}' "iostat_${prog}_${n}_${w}.txt")
            
            echo "${prog}_${n},${w},${CPU_VAL},${DISK_READ},${DISK_WRITE},${REAL_TIME}s" >> "$RESULTS"
            
            rm -f "iostat_${prog}_${n}_${w}.txt" "time_${prog}_${n}_${w}.txt"
        done
    done
done

cat "$RESULTS"
echo "✅ Part D complete! $RESULTS"
