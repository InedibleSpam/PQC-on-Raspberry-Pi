#!/bin/bash

# Set variables
DEVICE="Laptop"
ITERATIONS=5
OUTPUT="kem_${DEVICE}_${ITERATIONS}.csv"

# CSV header
echo "Device,Algorithm,Type,Operation,Iteration,Time" > "$OUTPUT"

# Run loop
for i in $(seq 1 $ITERATIONS)
do
    ./pqc_kem_test | while read line
    do
        # Extract numeric time value
        TIME=$(echo "$line" | grep -oE '[0-9]+\.[0-9]+')

        if [[ $line == *"KeyGen"* ]]; then
            echo "$DEVICE,Kyber768,PQC,KeyGen,$i,$TIME" >> "$OUTPUT"

        elif [[ $line == *"Encaps"* ]]; then
            echo "$DEVICE,Kyber768,PQC,Encaps,$i,$TIME" >> "$OUTPUT"

        elif [[ $line == *"Decaps"* ]]; then
            echo "$DEVICE,Kyber768,PQC,Decaps,$i,$TIME" >> "$OUTPUT"
        fi

    done
done

echo "Done. Results saved to $OUTPUT"
