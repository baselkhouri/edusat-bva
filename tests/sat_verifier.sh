#!/bin/bash
# Run this from tests root directory

# Check if WORKDIR is set
if [[ -z "$WORKDIR" ]]; then
    echo "Error: WORKDIR is not defined. Please set WORKDIR before running the script."
    return 1
fi

TEST_ROOT=$WORKDIR/tests

# Argments check
if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]; then
    echo "Usage: $0 <test_folder> [timeout_in_seconds]"
    return 1
fi

TEST_FOLDER=$1
# Set timeout in seconds, default to 3 if not provided
TIMEOUT_IN_SEC=${2:-3}

# Define color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
ORANGE='\033[0;33m'
NC='\033[0m' # No Color

# Counters
FAILED=0
NUM_TESTS=0
TIMEOUT=0

# Find edusat binary in ../build directory
BINARY=$(find $WORKDIR/build/ -name "edusat" -type f)
# Check if binary was found
if [ -z "$BINARY" ]; then
    echo "Error: 'edusat' binary not found. Try running 'make' in the project root directory."
    return 1
fi

CADICAL=$(find $WORKDIR/extern/cadical/build -name "cadical" -type f)

# Check if binary was found
if [ -z "$DRATTRIM" ]; then
    echo "Error: 'cadical' binary not found."
    return 1
fi

# Store the list of .cnf files in a variable
CNF_FILES=$(find "$WORKDIR/tests/$TEST_FOLDER" -name "*yes*.cnf")
ASS_FILE="assignment.txt"
BVA_EXPORT="bva_export.cnf"

# Check if any files were found
if [[ -z "$CNF_FILES" ]]; then
    echo "Error: No SAT .cnf files found in $WORKDIR/tests/$TEST_FOLDER."
    return 1  # Stop execution safely
else
    echo "Found $(echo "$CNF_FILES" | wc -l) SAT .cnf files."
fi

# Loop through the files safely
while IFS= read -r TEST_INPUT; do
    NUM_TESTS=$((NUM_TESTS+1))
    OUTPUT=$(timeout $TIMEOUT_IN_SEC $BINARY -bva -bva-export $BVA_EXPORT $TEST_INPUT)

    if [[ $? -eq 124 ]]; then
        echo -e "[${ORANGE}TIMEOUT${NC}] $TEST_INPUT: Timed out after ${TIMEOUT_IN_SEC} seconds."
        TIMEOUT=$((TIMEOUT+1))
        continue
    fi

    # If the output contains UNSAT or TIMEOUT
    if [[ $OUTPUT == *"UNSAT"* || $OUTPUT == *"TIMEOUT"* ]]; then
        echo -e "[${RED}FAILED${NC}] $TEST_INPUT: Unexpected result. Should be SAT."
        FAILED=$((FAILED+1))
        continue
    fi

    # In BVA_EXPORT find the line p cnf x y
    LINE=$(grep -m 1 "^p cnf" "$BVA_EXPORT")
    VARS=$(echo "$LINE" | awk '{print $3}')
    CLAUSES=$(echo "$LINE" | awk '{print $4}')

    # Check ASS_FILE contains lines exactly as VARS
    if [ $(wc -l < "$ASS_FILE") -ne "$VARS" ]; then
        echo -e "[${RED}FAILED${NC}] $BVA_EXPORT: Assignment file does not contain the correct number of variables."
        FAILED=$((FAILED+1))
        continue
    fi

    # Concatenate each line in ASS_FILE to the end of BVA_EXPORT and a whitespace followed by 0 to the end of each line
    echo >> "$BVA_EXPORT"

    while IFS= read -r LINE; do
        # Trim trailing spaces and append '0'
        echo "$(echo "$LINE" | awk '{$1=$1}1') 0" >> "$BVA_EXPORT"
    done < "$ASS_FILE"

    # Change the number of clauses in the problem line by an extra VARS
    #! Assignment file could contain lines with 0 (Unassigned vars).
    #! These variables don't appear in the CNF, currently, these lines cause a problem
    perl -i -p -e "s/^p cnf $VARS $CLAUSES/p cnf $VARS $((CLAUSES + VARS))/" "$BVA_EXPORT"
    
    # Give BVA_EXPORT to cadical, the output should be SATISFIABLE
    CADICAL_OUTPUT=$($CADICAL "$BVA_EXPORT")
    if [[ $CADICAL_OUTPUT != *"s SATISFIABLE"* ]]; then
        echo -e "[${RED}FAILED${NC}] $TEST_INPUT: Cadical failed to verify assignment."
        FAILED=$((FAILED+1))
    else
        echo -e "[${GREEN}PASSED${NC}] $TEST_INPUT: Cadical successfully verified the assignment"
    fi
done <<< "$CNF_FILES"

# Delete tmp.cnf file
rm -f $BVA_EXPORT $ASS_FILE

# Print the number of failed tests
echo
echo "==================== Summary - sat_verifier.sh ===================="
if [ $FAILED -eq 0 ] && [ $TIMEOUT -eq 0 ]; then
    echo -e "All tests: ${GREEN}PASSED${NC}"
else
    if [ $FAILED -ne 0 ]; then
        echo -e "$FAILED/$NUM_TESTS tests ${RED}FAILED${NC}"
    fi
    if [ $TIMEOUT -ne 0 ]; then
        echo -e "$TIMEOUT/$NUM_TESTS tests ${ORANGE}TIMED OUT${NC} (timeout: ${TIMEOUT_IN_SEC}s)"
    fi
fi
echo "==================================================================="
