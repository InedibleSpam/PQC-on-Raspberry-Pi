#!/bin/bash

DEVICE="Laptop"
ITERATIONS=5
OUTPUT="sig_${DEVICE}_${ITERATIONS}.csv"

# CSV header
echo "Device,Algorithm,Type,Operation,Iteration,Time" > "$OUTPUT"

for i in $(seq 1 $ITERATIONS)
do
    OUTPUT_TEXT=$(./pqc_sig_test)

    echo "$OUTPUT_TEXT" | while read line
    do
        # Extract numeric value safely
        TIME=$(echo "$line" | grep -oE '[0-9]+\.[0-9]+')

        if [[ $line == *"KeyGen"* ]]; then
            echo "$DEVICE,ML-DSA-44,PQC,KeyGen,$i,$TIME" >> "$OUTPUT"

        elif [[ $line == *"Sign"* ]]; then
            echo "$DEVICE,ML-DSA-44,PQC,Sign,$i,$TIME" >> "$OUTPUT"

        elif [[ $line == *"Verify"* ]]; then
            echo "$DEVICE,ML-DSA-44,PQC,Verify,$i,$TIME" >> "$OUTPUT"
        fi

    done
done

echo "Done. Results saved to $OUTPUT"
