#!/bin/bash

# Configuration settings for your current test run
DEVICE="Laptop"
ITERATIONS=5
OUTPUT="classical_comparison_${DEVICE}.csv"

# Initialize the CSV file with headers
echo "Device,Algorithm,Type,Operation,Iteration,Time" > "$OUTPUT"

echo "Starting Classical Benchmarks (ECDH, ECDSA, RSA)..."

for i in $(seq 1 $ITERATIONS)
do
    echo "Running Iteration $i..."
    
    ./classical_test | while read line
    do
        # Extract the high-precision time value from the C output
        TIME=$(echo "$line" | grep -oE '[0-9]+\.[0-9]+')

        # 1. IDENTIFY ALGORITHM
        # Logic updated to distinguish between Exchange (ECDH) and Signing (ECDSA)
        if [[ $line == *"ECDH"* ]]; then
            ALG="ECDH-P256"
        elif [[ $line == *"ECDSA"* ]]; then
            ALG="ECDSA-P256"
        elif [[ $line == *"RSA"* ]]; then
            ALG="RSA-3072"
        else
            # Skip noise like header text or verification results
            continue
        fi

        # 2. IDENTIFY OPERATION AND LOG TO CSV
        if [[ $line == *"KeyGen"* ]]; then
            echo "$DEVICE,$ALG,Classical,KeyGen,$i,$TIME" >> "$OUTPUT"
            
        elif [[ $line == *"Derivation"* ]]; then
            echo "$DEVICE,$ALG,Classical,Derivation,$i,$TIME" >> "$OUTPUT"
            
        elif [[ $line == *"Sign"* ]]; then
            echo "$DEVICE,$ALG,Classical,Sign,$i,$TIME" >> "$OUTPUT"
            
        elif [[ $line == *"Verify"* ]]; then
            echo "$DEVICE,$ALG,Classical,Verify,$i,$TIME" >> "$OUTPUT"
        fi
    done
done

echo "Done! Classical results saved to $OUTPUT"