#!/bin/bash

# =========================================================
# CONFIGURATION SECTION
# =========================================================

# Label for dataset (Laptop vs Raspberry Pi)
DEVICE="Laptop"

# Number of repeated runs for statistical stability
ITERATIONS=5

# Output CSV file name
OUTPUT="pqc_kem_comparison_${DEVICE}_${ITERATIONS}.csv"

# CSV header with extended metrics (Time + Memory + CPU)
echo "Device,Algorithm,Type,Operation,Iteration,Time,MemoryKB,CPU" > "$OUTPUT"

echo "Starting PQC KEM Benchmarks (Kyber, BIKE, McEliece)..."

# =========================================================
# MAIN EXPERIMENT LOOP
# =========================================================

for i in $(seq 1 $ITERATIONS)
do
    echo "Running iteration $i..."

    # =====================================================
    # Run PQC test WITH system resource tracking
    # /usr/bin/time -v provides:
    #   - program output (stderr redirected)
    #   - memory usage
    #   - CPU usage
    # =====================================================

    OUTPUT_TEXT=$(/usr/bin/time -v ./pqc_kem_test 2>&1)

    # =====================================================
    # Extract peak memory usage (very important for embedded systems)
    # =====================================================

    MEM=$(echo "$OUTPUT_TEXT" | grep "Maximum resident set size" | awk '{print $6}')

    # =====================================================
    # Extract CPU usage percentage
    # =====================================================

    CPU=$(echo "$OUTPUT_TEXT" | grep "Percent of CPU this job got" | awk '{print $7}')

    # =====================================================
    # Process line-by-line output from PQC test program
    # =====================================================

    echo "$OUTPUT_TEXT" | while read line
    do
        # -------------------------------------------------
        # Extract numeric timing value (seconds)
        # -------------------------------------------------
        TIME=$(echo "$line" | grep -oE '[0-9]+\.[0-9]+')

        # -------------------------------------------------
        # Normalize text for algorithm detection
        # (handles uppercase/lowercase differences)
        # -------------------------------------------------
        LOWER_LINE=$(echo "$line" | tr '[:upper:]' '[:lower:]')

        # -------------------------------------------------
        # Identify PQC algorithm
        # -------------------------------------------------
        if [[ "$LOWER_LINE" == *"kyber768"* ]] || [[ "$LOWER_LINE" == *"kyber 768"* ]]; then
            ALG="Kyber-768"

        elif [[ "$LOWER_LINE" == *"bike"* ]]; then
            ALG="BIKE-L1"

        elif [[ "$LOWER_LINE" == *"mceliece"* ]]; then
            ALG="Classic-McEliece"

        else
            # Skip lines that are not algorithm-related
            continue
        fi

        # -------------------------------------------------
        # Identify operation type
        # -------------------------------------------------
        if [[ "$line" == *"KeyGen"* ]]; then
            OP="KeyGen"

        elif [[ "$line" == *"Encapsulation"* ]]; then
            OP="Encapsulation"

        elif [[ "$line" == *"Decapsulation"* ]]; then
            OP="Decapsulation"

        else
            # Skip irrelevant output lines
            continue
        fi

        # -------------------------------------------------
        # Write structured row into CSV
        # Format:
        # Device,Algorithm,Type,Operation,Iteration,Time,MemoryKB,CPU
        # -------------------------------------------------
        echo "$DEVICE,$ALG,PQC,$OP,$i,$TIME,$MEM,$CPU" >> "$OUTPUT"

    done
done

# =========================================================
# FINAL OUTPUT MESSAGE
# =========================================================

echo "Done! PQC results saved to $OUTPUT"
