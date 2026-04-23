#!/bin/bash

# Configuration: Update DEVICE to reflect your current hardware (Pi 5 or Laptop)
DEVICE="RaspberryPi_5"
ITERATIONS=1000
OUTPUT="pqc_sig_${DEVICE}_${ITERATIONS}.csv"

# Header includes 'CommSize' for the MTU/Network analysis
echo "Device,Algorithm,Type,Operation,Iteration,Time,MemoryKB,CPU,CommSize" > "$OUTPUT"

echo "Starting PQC Signature Benchmarks..."

for i in $(seq 1 $ITERATIONS)
do
    echo "Running iteration $i..."

    # Capture system profiling data (RSS Memory and CPU %)
    OUTPUT_TEXT=$(/usr/bin/time -v ./pqc_sig_test 2>&1)

    # Extract Peak RAM usage (Maximum Resident Set Size)
    MEM=$(echo "$OUTPUT_TEXT" | grep "Maximum resident set size" | awk '{print $6}')
    
    # Extract CPU utilization
    CPU=$(echo "$OUTPUT_TEXT" | grep "Percent of CPU this job got" | awk '{print $7}')

    # Extract CommSize for each signature scheme to analyze 'Bandwidth Tax'
    ML_COMM=$(echo "$OUTPUT_TEXT" | grep "ml_dsa_44 Comm Size" | grep -oP 'Total=\K[0-9]+')
    FALCON_COMM=$(echo "$OUTPUT_TEXT" | grep "falcon_512 Comm Size" | grep -oP 'Total=\K[0-9]+')
    SLH_COMM=$(echo "$OUTPUT_TEXT" | grep "slh_dsa_pure_sha2_128f Comm Size" | grep -oP 'Total=\K[0-9]+')

    echo "$OUTPUT_TEXT" | while read line
    do
        # Extract numeric timing (seconds)
        TIME=$(echo "$line" | grep -oE '[0-9]+\.[0-9]+')
        [[ -z "$TIME" ]] && continue 

        LOWER_LINE=$(echo "$line" | tr '[:upper:]' '[:lower:]')

        # Map current line to Algorithm and its static Communication Size
        if [[ "$LOWER_LINE" == *"ml_dsa_44"* ]]; then
            ALG="ML-DSA-44"; COMM=$ML_COMM
        elif [[ "$LOWER_LINE" == *"falcon_512"* ]]; then
            ALG="Falcon-512"; COMM=$FALCON_COMM
        elif [[ "$LOWER_LINE" == *"slh_dsa"* ]]; then
            ALG="SLH-DSA-128f"; COMM=$SLH_COMM
        else
            continue
        fi

        # Identify the Operation
        if [[ "$line" == *"KeyGen"* ]]; then OP="KeyGen"
        elif [[ "$line" == *"Sign"* ]] && [[ "$line" != *"KeyGen"* ]]; then OP="Sign"
        elif [[ "$line" == *"Verify"* ]]; then OP="Verify"
        else continue; fi

        # Write consolidated row to CSV for paper analysis
        echo "$DEVICE,$ALG,PQC,$OP,$i,$TIME,$MEM,$CPU,$COMM" >> "$OUTPUT"
    done
done

echo "PQC Signature Benchmarks Complete. Data saved to $OUTPUT"