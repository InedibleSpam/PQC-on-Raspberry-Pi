#!/bin/bash

DEVICE="Raspberry Pi"
ITERATIONS=1000
OUTPUT="classical_${DEVICE}_benchmarks.csv"

# Added CommSize column to reflect Network Viability
echo "Device,Algorithm,Type,Operation,Iteration,Time,MemoryKB,CPU,CommSize" > "$OUTPUT"

echo "Starting Benchmarks..."

for i in $(seq 1 $ITERATIONS)
do
    echo "Iteration $i"
    OUTPUT_TEXT=$(/usr/bin/time -v ./classical_test 2>&1)
    
    MEM=$(echo "$OUTPUT_TEXT" | grep "Maximum resident set size" | awk '{print $6}')
    CPU=$(echo "$OUTPUT_TEXT" | grep "Percent of CPU this job got" | awk '{print $7}')

    # Capture the Comm Sizes into variables to associate with their algorithms
    ECDH_COMM=$(echo "$OUTPUT_TEXT" | grep "ECDH Comm Size" | grep -oP 'Total=\K[0-9]+')
    ECDSA_COMM=$(echo "$OUTPUT_TEXT" | grep "ECDSA Comm Size" | grep -oP 'Total=\K[0-9]+')
    RSA_COMM=$(echo "$OUTPUT_TEXT" | grep "RSA Comm Size" | grep -oP 'Total=\K[0-9]+')

    echo "$OUTPUT_TEXT" | while read line
    do
        TIME=$(echo "$line" | grep -oE '[0-9]+\.[0-9]+')
        [[ -z "$TIME" ]] && continue

        if [[ $line == *"ECDH"* ]]; then
            ALG="ECDH-P256"; COMM=$ECDH_COMM
        elif [[ $line == *"ECDSA"* ]]; then
            ALG="ECDSA-P256"; COMM=$ECDSA_COMM
        elif [[ $line == *"RSA"* ]]; then
            ALG="RSA-3072"; COMM=$RSA_COMM
        else
            continue
        fi

        if [[ $line == *"KeyGen"* ]]; then OP="KeyGen"
        elif [[ $line == *"Derivation"* ]]; then OP="Derivation"
        elif [[ $line == *"Sign"* ]]; then OP="Sign"
        elif [[ $line == *"Verify"* ]]; then OP="Verify"
        else continue; fi

        echo "$DEVICE,$ALG,Classical,$OP,$i,$TIME,$MEM,$CPU,$COMM" >> "$OUTPUT"
    done
done
echo "Results saved to $OUTPUT"
