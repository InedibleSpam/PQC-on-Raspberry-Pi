#!/bin/bash

DEVICE="Raspberry PI"
ITERATIONS=1000
OUTPUT="classical_${DEVICE}_${ITERATIONS}.csv"

echo "Device,Algorithm,Type,Operation,Iteration,Time" > "$OUTPUT"

for i in $(seq 1 $ITERATIONS)
do
    ./classical_test | while read line
    do

        # Extract ONLY the number safely (works no matter formatting)
        TIME=$(echo "$line" | grep -oE '[0-9]+\.[0-9]+')

        if [[ $line == *"ECDH KeyGen"* ]]; then
            echo "$DEVICE,ECDH,Classical,KeyGen,$i,$TIME" >> "$OUTPUT"

        elif [[ $line == *"ECDH Shared Secret"* ]]; then
            echo "$DEVICE,ECDH,Classical,SharedSecret,$i,$TIME" >> "$OUTPUT"

        elif [[ $line == *"ECDSA Sign"* ]]; then
            echo "$DEVICE,ECDSA,Classical,Sign,$i,$TIME" >> "$OUTPUT"

        elif [[ $line == *"ECDSA Verify"* ]]; then
            echo "$DEVICE,ECDSA,Classical,Verify,$i,$TIME" >> "$OUTPUT"
        fi

    done
done

echo "Done. Results saved to $OUTPUT"
