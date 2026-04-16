#!/bin/bash

DEVICE="Laptop"
ITERATIONS=5
OUTPUT="pqc_kem_comparison_${DEVICE}.csv"

echo "Device,Algorithm,Type,Operation,Iteration,Time" > "$OUTPUT"

echo "Starting PQC KEM Benchmarks..."

for i in $(seq 1 $ITERATIONS)
do
    echo "Running Iteration $i..."
    
    ./pqc_kem_test | while read line
    do
        # Extract the time (decimal number)
        TIME=$(echo "$line" | grep -oE '[0-9]+\.[0-9]+')

        # 1. IDENTIFY ALGORITHM (Matching exact liboqs strings)
        # We convert to lowercase to handle "Kyber768", "kyber768", etc.
        LOWER_LINE=$(echo "$line" | tr '[:upper:]' '[:lower:]')

        if [[ "$LOWER_LINE" == *"kyber768"* ]] || [[ "$LOWER_LINE" == *"kyber 768"* ]]; then
            ALG="Kyber-768"
        elif [[ "$LOWER_LINE" == *"bike"* ]]; then
            ALG="BIKE-L1"
        elif [[ "$LOWER_LINE" == *"mceliece"* ]]; then
            ALG="Classic-McEliece"
        else
            continue 
        fi

        # 2. IDENTIFY OPERATION AND LOG
        if [[ "$line" == *"KeyGen"* ]]; then
            echo "$DEVICE,$ALG,PQC,KeyGen,$i,$TIME" >> "$OUTPUT"
            
        elif [[ "$line" == *"Encapsulation"* ]]; then
            echo "$DEVICE,$ALG,PQC,Encapsulation,$i,$TIME" >> "$OUTPUT"
            
        elif [[ "$line" == *"Decapsulation"* ]]; then
            echo "$DEVICE,$ALG,PQC,Decapsulation,$i,$TIME" >> "$OUTPUT"
        fi
    done
done

echo "Done! Check your CSV for Kyber-768 entries now."