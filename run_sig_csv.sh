#!/bin/bash

# =========================================================
# CONFIGURATION SECTION
# =========================================================

# Label for dataset comparison (Laptop vs Raspberry Pi)
DEVICE="Laptop"

# Number of repeated runs for statistical reliability
ITERATIONS=5

# Output CSV file name
OUTPUT="pqc_sig_comparison_${DEVICE}_full.csv"

# CSV header (now includes Memory + CPU for embedded analysis)
echo "Device,Algorithm,Type,Operation,Iteration,Time,MemoryKB,CPU" > "$OUTPUT"

echo "Starting PQC Signature Benchmarks (Dilithium, Falcon, SLH-DSA)..."

# =========================================================
# MAIN EXPERIMENT LOOP
# =========================================================

for i in $(seq 1 $ITERATIONS)
do
    echo "Running iteration $i..."

    # =====================================================
    # Run signature test WITH system profiling
    # /usr/bin/time -v gives memory + CPU + output
    # =====================================================

    OUTPUT_TEXT=$(/usr/bin/time -v ./pqc_sig_test 2>&1)

    # =====================================================
    # Extract memory usage (peak RAM usage)
    # =====================================================

    MEM=$(echo "$OUTPUT_TEXT" | grep "Maximum resident set size" | awk '{print $6}')

    # =====================================================
    # Extract CPU usage percentage
    # =====================================================

    CPU=$(echo "$OUTPUT_TEXT" | grep "Percent of CPU this job got" | awk '{print $7}')

    # =====================================================
    # Process program output line-by-line
    # =====================================================

    echo "$OUTPUT_TEXT" | while read line
    do
        # -------------------------------------------------
        # Extract numeric timing value
        # -------------------------------------------------
        TIME=$(echo "$line" | grep -oE '[0-9]+\.[0-9]+')

        # -------------------------------------------------
        # Normalize text for safe matching
        # -------------------------------------------------
        LOWER_LINE=$(echo "$line" | tr '[:upper:]' '[:lower:]')

        # -------------------------------------------------
        # Identify PQC signature algorithm
        # -------------------------------------------------
        if [[ "$LOWER_LINE" == *"ml"* && "$LOWER_LINE" == *"dsa"* ]] || [[ "$LOWER_LINE" == *"dilithium"* ]]; then
            ALG="ML-DSA-44"

        elif [[ "$LOWER_LINE" == *"falcon"* ]]; then
            ALG="Falcon-512"

        elif [[ "$LOWER_LINE" == *"slh"* ]] || [[ "$LOWER_LINE" == *"sphincs"* ]]; then
            ALG="SLH-DSA-128f"

        else
            # Skip irrelevant lines
            continue
        fi

        # -------------------------------------------------
        # Identify operation type
        # -------------------------------------------------
        if [[ "$line" == *"KeyGen"* ]]; then
            OP="KeyGen"

        elif [[ "$line" == *"Sign"* ]] && [[ "$line" != *"KeyGen"* ]]; then
            OP="Sign"

        elif [[ "$line" == *"Verify"* ]]; then
            OP="Verify"

        else
            continue
        fi

        # -------------------------------------------------
        # Write structured row to CSV
        # Format:
        # Device,Algorithm,Type,Operation,Iteration,Time,MemoryKB,CPU
        # -------------------------------------------------
        echo "$DEVICE,$ALG,PQC,$OP,$i,$TIME,$MEM,$CPU" >> "$OUTPUT"

    done
done

# =========================================================
# FINAL MESSAGE
# =========================================================

echo "Done! PQC signature results saved to $OUTPUT"