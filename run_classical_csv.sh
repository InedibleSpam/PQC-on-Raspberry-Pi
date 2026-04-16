#!/bin/bash

<<<<<<< HEAD
DEVICE="Raspberry PI"
ITERATIONS=1000
=======
# Set variables FIRST
DEVICE="Laptop"
ITERATIONS=5
>>>>>>> 136e95fecfafb10ca69c2009c1a7e66d7022e35b
OUTPUT="classical_${DEVICE}_${ITERATIONS}.csv"

# Create CSV with header
echo "Device,Algorithm,Type,Operation,Iteration,Time" > "$OUTPUT"

# Loop runs
for i in $(seq 1 $ITERATIONS)
do
    ./classical_test | while read line
    do
        # Extract number safely
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
