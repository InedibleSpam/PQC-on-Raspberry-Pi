#!/bin/bash

DEVICE="Laptop"
ITERATIONS=5
OUTPUT="pqc_sig_comparison_${DEVICE}.csv"

# 1. CREATE CSV HEADER
echo "Device,Algorithm,Type,Operation,Iteration,Time" > "$OUTPUT"

echo "Starting PQC Signature Benchmarks..."

for i in $(seq 1 $ITERATIONS)
do
    echo "Running Iteration $i..."
    
    ./pqc_sig_test | while read line
    do
        # Extract the high-precision time
        TIME=$(echo "$line" | grep -oE '[0-9]+\.[0-9]+')

        # 2. IDENTIFY ALGORITHM (More aggressive matching)
        # We look for "ml", "dsa", or "dilithium" to ensure we don't miss it
        LOWER_LINE=$(echo "$line" | tr '[:upper:]' '[:lower:]')

        if [[ "$LOWER_LINE" == *"ml"* && "$LOWER_LINE" == *"dsa"* ]] || [[ "$LOWER_LINE" == *"dilithium"* ]]; then
            ALG="ML-DSA-44"
        elif [[ "$LOWER_LINE" == *"falcon"* ]]; then
            ALG="Falcon-512"
        elif [[ "$LOWER_LINE" == *"slh"* ]] || [[ "$LOWER_LINE" == *"sphincs"* ]]; then
            ALG="SLH-DSA-128f"
        else
            continue 
        fi

        # 3. IDENTIFY OPERATION AND LOG
        if [[ "$line" == *"KeyGen"* ]]; then
            echo "$DEVICE,$ALG,PQC,KeyGen,$i,$TIME" >> "$OUTPUT"
            
        elif [[ "$line" == *"Sign"* ]] && [[ "$line" != *"KeyGen"* ]]; then
            echo "$DEVICE,$ALG,PQC,Sign,$i,$TIME" >> "$OUTPUT"
            
        elif [[ "$line" == *"Verify"* ]]; then
            echo "$DEVICE,$ALG,PQC,Verify,$i,$TIME" >> "$OUTPUT"
        fi
    done
done

echo "Done! Check your CSV for ML-DSA entries now."