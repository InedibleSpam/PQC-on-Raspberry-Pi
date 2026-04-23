#!/bin/bash

# Configuration: Set the device name for data labeling in the final paper
DEVICE="RaspberryPi_5"
ITERATIONS=1000 # Higher iterations reduce statistical 'noise' from OS background tasks
OUTPUT="classical_${DEVICE}_${ITERATIONS}.csv"

# Header: Added 'CommSize' to the columns for Network Viability analysis
echo "Device,Algorithm,Type,Operation,Iteration,Time,MemoryKB,CPU,CommSize" > "$OUTPUT"

echo "Beginning Benchmarks for $DEVICE..."

for i in $(seq 1 $ITERATIONS)
do
    echo "Processing Iteration $i..."
    
    # Run the C binary using the 'time -v' wrapper to capture System-Level metrics
    # stderr is redirected to stdout (2>&1) so we can parse it all in one variable
    OUTPUT_TEXT=$(/usr/bin/time -v ./classical_test 2>&1)
    
    # RSS Capture: Grabs the peak physical RAM usage (High Water Mark)
    # This is used to establish the 'Memory Baseline'
    MEM=$(echo "$OUTPUT_TEXT" | grep "Maximum resident set size" | awk '{print $6}')
    
    # CPU Capture: Grabs the efficiency of the execution on the ARM SoC
    CPU=$(echo "$OUTPUT_TEXT" | grep "Percent of CPU this job got" | awk '{print $7}')

    # CommSize Capture: Extracts the 'Total Bytes' printed by the C program
    # Uses grep/regex to pull just the numbers after 'Total='
    ECDH_COMM=$(echo "$OUTPUT_TEXT" | grep "ECDH Comm Size" | grep -oP 'Total=\K[0-9]+')
    ECDSA_COMM=$(echo "$OUTPUT_TEXT" | grep "ECDSA Comm Size" | grep -oP 'Total=\K[0-9]+')
    RSA_COMM=$(echo "$OUTPUT_TEXT" | grep "RSA Comm Size" | grep -oP 'Total=\K[0-9]+')

    # Parsing Loop: Processes each line of output to categorize metrics by Algorithm/Operation
    echo "$OUTPUT_TEXT" | while read line
    do
        # Use regex to pull floating point numbers (the seconds) from the line
        TIME=$(echo "$line" | grep -oE '[0-9]+\.[0-9]+')
        [[ -z "$TIME" ]] && continue # Skip lines that don't have a time value

        # Mapping Logic: Assign Algorithm Name and the corresponding CommSize
        if [[ $line == *"ECDH"* ]]; then
            ALG="ECDH-P256"; COMM=$ECDH_COMM
        elif [[ $line == *"ECDSA"* ]]; then
            ALG="ECDSA-P256"; COMM=$ECDSA_COMM
        elif [[ $line == *"RSA"* ]]; then
            ALG="RSA-3072"; COMM=$RSA_COMM
        else
            continue # Ignore header or debug lines
        fi

        # Mapping Logic: Assign Operation type for easier filtering in Excel/R
        if [[ $line == *"KeyGen"* ]]; then OP="KeyGen"
        elif [[ $line == *"Derivation"* ]]; then OP="Derivation"
        elif [[ $line == *"Sign"* ]]; then OP="Sign"
        elif [[ $line == *"Verify"* ]]; then OP="Verify"
        else continue; fi

        # Write the consolidated data row into the CSV file
        echo "$DEVICE,$ALG,Classical,$OP,$i,$TIME,$MEM,$CPU,$COMM" >> "$OUTPUT"
    done
done

echo "Benchmarks complete. Data written to: $OUTPUT"