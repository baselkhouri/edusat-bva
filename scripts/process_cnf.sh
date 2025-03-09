#!/bin/bash

# This script goes over all the CNF files in a specific folder
# Runs cadical on each file and checks if the output contains "SATISFIABLE" or "UNSATISFIABLE"
# Copies the file to a new directory with a suffix "_yes.cnf" or "_no.cnf" based on the output
# If unknown output, copies the file with a suffix "_unknown.cnf"

# Check if WORKDIR is set
if [[ -z "$WORKDIR" ]]; then
    echo "Error: WORKDIR is not defined. Please set WORKDIR before running the script."
    exit 1
fi

# Define directories (hard coded for now)
INPUT_DIR="$WORKDIR/tests/hard_cnf_instances_old"
OUTPUT_DIR="$WORKDIR/tests/hard_cnf_instances"

# Ensure the input directory exists
if [[ ! -d "$INPUT_DIR" ]]; then
    echo "Error: Directory $INPUT_DIR does not exist."
    exit 1
fi

# Ensure the output directory exists
mkdir -p "$OUTPUT_DIR"

# Process each .cnf file
for file in "$INPUT_DIR"/*.cnf; do
    [[ -e "$file" ]] || continue  # Skip if no files match

    echo "Processing: $file"
    
    # Run cadical and capture output
    result=$(cadical -n -q "$file" 2>&1)

    # Determine new file name
    if echo "$result" | grep -q "UNSATISFIABLE"; then
        new_name="${file%.cnf}_no.cnf"
    elif echo "$result" | grep -q "SATISFIABLE"; then
        new_name="${file%.cnf}_yes.cnf"
    else
        new_name="${file%.cnf}_unknown.cnf"
    fi

    cp "$file" "$OUTPUT_DIR/$(basename "$new_name")"
    echo "Copied to: $OUTPUT_DIR/$(basename "$new_name")"
done

echo "Processing complete!"
