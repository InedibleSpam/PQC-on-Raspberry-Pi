#!/bin/bash

# =========================================================
# CONFIGURATION SECTION
# =========================================================

# Label for dataset (used to compare Laptop vs Raspberry Pi)
DEVICE="Laptop"

# Number of times to repeat the experiment for statistical accuracy
ITERATIONS=5

# Output CSV file name
OUTPUT="classical_${DEVICE}_full.csv"

# Create CSV file and write header row
# Added MemoryKB and CPU columns for embedded systems analysis
echo "Device,Algorithm,Type,Operation,Iteration,Time,MemoryKB,CPU" > "$OUTPUT"

echo "Starting Classical Crypto Benchmarks (with memory + CPU tracking)..."

# =========================================================
# MAIN EXPERIMENT LOOP
# =========================================================

for i in $(seq 1 $ITERATIONS)
do
    echo "Running iteration $i"

    # =====================================================
    # Run program WITH resource tracking enabled
    # /usr/bin/time -v gives:
    #   - execution output (stderr)
    #   - memory usage
    #   - CPU usage
    # =====================================================

    OUTPUT_TEXT=$(/usr/bin/time -v ./classical_test 2>&1)

    # =====================================================
    # Extract memory usage (peak RAM used)
    # This is critical for embedded system viability
    # =====================================================

    MEM=$(echo "$OUTPUT_TEXT" | grep "Maximum resident set size" | awk '{print $6}')

    # =====================================================
    # Extract CPU usage percentage
    # Shows how efficiently the CPU was used
    # =====================================================

    CPU=$(echo "$OUTPUT_TEXT" | grep "Percent of CPU this job got" | awk '{print $7}')

    # =====================================================
    # Process each line of program output
    # Each line corresponds to one cryptographic operation
    # =====================================================

    echo "$OUTPUT_TEXT" | while read line
    do
        # -------------------------------------------------
        # Extract numeric time value from the line
        # Example: "ECDSA Sign time: 0.000123 seconds"
        # regex pulls only the number
        # -------------------------------------------------
        TIME=$(echo "$line" | grep -oE '[0-9]+\.[0-9]+')

        # -------------------------------------------------
        # Identify which algorithm produced this line
        # -------------------------------------------------
        if [[ $line == *"ECDH"* ]]; then
            ALG="ECDH-P256"

        elif [[ $line == *"ECDSA"* ]]; then
            ALG="ECDSA-P256"

        elif [[ $line == *"RSA"* ]]; then
            ALG="RSA-3072"

        else
            # Skip non-matching lines (headers, debug text, etc.)
            continue
        fi

        # -------------------------------------------------
        # Identify operation type (what crypto step it was)
        # -------------------------------------------------
        if [[ $line == *"KeyGen"* ]]; then
            OP="KeyGen"

        elif [[ $line == *"Derivation"* ]]; then
            OP="Derivation"

        elif [[ $line == *"Sign"* ]]; then
            OP="Sign"

        elif [[ $line == *"Verify"* ]]; then
            OP="Verify"

        else
            # Ignore irrelevant lines
            continue
        fi

        # -------------------------------------------------
        # Write structured row into CSV file
        #
        # Format:
        # Device,Algorithm,Type,Operation,Iteration,Time,MemoryKB,CPU
        # -------------------------------------------------
        echo "$DEVICE,$ALG,Classical,$OP,$i,$TIME,$MEM,$CPU" >> "$OUTPUT"

    done
done

# =========================================================
# FINAL MESSAGE
# =========================================================

echo "Done! Results saved to $OUTPUT"