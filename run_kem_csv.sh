#!/bin/bash

# Configuration for labeling and statistical depth
DEVICE="RaspberryPi_5"
ITERATIONS=1000
OUTPUT="pqc_kem_${DEVICE}_${ITERATIONS}.csv"


# CSV Header: Matches the Classical structure for easy merging later
echo "Device,Algorithm,Type,Operation,Iteration,Time,MemoryKB,CPU,CommSize" > "$OUTPUT"

echo "Starting PQC KEM Benchmarks..."
# CS

for i in $(seq 1 $ITERATIONS)
do
    echo "Running iteration $i..."

    # Capture system metrics (RSS/CPU) using time -v
    OUTPUT_TEXT=$(/usr/bin/time -v ./pqc_kem_test 2>&1)

    # Extract Peak RSS (Maximum Resident Set Size)
    MEM=$(echo "$OUTPUT_TEXT" | grep "Maximum resident set size" | awk '{print $6}')
    
    # Extract CPU Load percentage
    CPU=$(echo "$OUTPUT_TEXT" | grep "Percent of CPU this job got" | awk '{print $7}')

    # Extract Algorithm-specific CommSize footprints
    # Uses regex to find the 'Total=' value for each specific algorithm
    KYBER_COMM=$(echo "$OUTPUT_TEXT" | grep "kyber_768 Comm Size" | grep -oP 'Total=\K[0-9]+')
    BIKE_COMM=$(echo "$OUTPUT_TEXT" | grep "bike_l1 Comm Size" | grep -oP 'Total=\K[0-9]+')
    MCELIECE_COMM=$(echo "$OUTPUT_TEXT" | grep "mceliece_348864 Comm Size" | grep -oP 'Total=\K[0-9]+')

    echo "$OUTPUT_TEXT" | while read line
    do
        # Extract timing values
        TIME=$(echo "$line" | grep -oE '[0-9]+\.[0-9]+')
        [[ -z "$TIME" ]] && continue 

        LOWER_LINE=$(echo "$line" | tr '[:upper:]' '[:lower:]')

        # Map Line to Algorithm and its corresponding Communication Size
        if [[ "$LOWER_LINE" == *"kyber_768"* ]]; then
            ALG="Kyber-768"; COMM=$KYBER_COMM
        elif [[ "$LOWER_LINE" == *"bike_l1"* ]]; then
            ALG="BIKE-L1"; COMM=$BIKE_COMM
        elif [[ "$LOWER_LINE" == *"mceliece"* ]]; then
            ALG="Classic-McEliece"; COMM=$MCELIECE_COMM
        else
            continue
        fi

        # Map Line to Operation
        if [[ "$line" == *"KeyGen"* ]]; then OP="KeyGen"
        elif [[ "$line" == *"Encapsulation"* ]]; then OP="Encapsulation"
        elif [[ "$line" == *"Decapsulation"* ]]; then OP="Decapsulation"
        else continue; fi

        # Write data to CSV
        echo "$DEVICE,$ALG,PQC,$OP,$i,$TIME,$MEM,$CPU,$COMM" >> "$OUTPUT"
    done
done

echo "PQC Benchmarks Complete. Data saved to $OUTPUT"

