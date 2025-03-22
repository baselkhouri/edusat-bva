#!/bin/bash
# Run this from tests root directory

# Check if WORKDIR is set
if [[ -z "$WORKDIR" ]]; then
    echo "Error: WORKDIR is not defined. Please set WORKDIR before running the script."
    return 1
fi

TEST_ROOT=$WORKDIR/tests

# Take test folder as argument
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <test_folder>"
    return 1
fi

TEST_FOLDER=$1

# Define color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Counters
FAILED=0
NUM_TESTS=0

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

# Check if any files were found
if [[ -z "$CNF_FILES" ]]; then
    echo "Error: No SAT .cnf files found in $WORKDIR/tests/$TEST_FOLDER."
    return 1  # Stop execution safely
else
    echo "Found $(echo "$CNF_FILES" | wc -l) SAT .cnf files."
fi

# Create tmp.cnf file
TMP_CNF="tmp.cnf"

# Loop through the files safely
while IFS= read -r TEST_INPUT; do
    NUM_TESTS=$((NUM_TESTS+1))
    OUTPUT=$(timeout 180 $BINARY -bva $TEST_INPUT)

    if [[ $? -eq 124 ]]; then
        echo -e "[${RED}TIMEOUT${NC}] $TEST_INPUT: Timed out after 3 minutes."
        FAILED=$((FAILED+1))
        continue
    fi

    # If the output contains UNSAT or TIMEOUT
    if [[ $OUTPUT == *"UNSAT"* || $OUTPUT == *"TIMEOUT"* ]]; then
        echo -e "[${RED}FAILED${NC}] $TEST_INPUT: Unexpected result. Should be SAT."
        FAILED=$((FAILED+1))
    fi

    # In TEST_INPUT find the line p cnf x y
    LINE=$(grep -m 1 "^p cnf" "$TEST_INPUT")
    VARS=$(echo "$LINE" | awk '{print $3}')
    CLAUSES=$(echo "$LINE" | awk '{print $4}')

    # Check ASS_FILE contains lines exactly as VARS
    if [ $(wc -l < "$ASS_FILE") -ne "$VARS" ]; then
        echo -e "[${RED}FAILED${NC}] $TEST_INPUT: Assignment file does not contain the correct number of variables."
        FAILED=$((FAILED+1))
    fi
    
    # Copy TEST_INPUT into TMP_CNF
    cp "$TEST_INPUT" "$TMP_CNF"

    # Concatenate each line in ASS_FILE to the end of TMP_CNF and a whitespace followed by 0 to the end of each line
    echo >> "$TMP_CNF"

    while IFS= read -r LINE; do
        # Trim trailing spaces and append '0'
        echo "$(echo "$LINE" | awk '{$1=$1}1') 0" >> "$TMP_CNF"
    done < "$ASS_FILE"

    # Change the number of clauses in the problem line by an extra VARS
    #! Assignment file could contain lines with 0 (Unassigned vars).
    #! These variables don't appear in the CNF, currently, these lines cause a problem
    perl -i -p -e "s/^p cnf $VARS $CLAUSES/p cnf $VARS $((CLAUSES + VARS))/" "$TMP_CNF"
    
    # Give TMP_CNF to cadical, the output should be SATISFIABLE
    CADICAL_OUTPUT=$($CADICAL "$TMP_CNF")
    if [[ $CADICAL_OUTPUT != *"s SATISFIABLE"* ]]; then
        echo -e "[${RED}FAILED${NC}] $TEST_INPUT: Cadical failed to verify assignment."
        FAILED=$((FAILED+1))
    else
        echo -e "[${GREEN}PASSED${NC}] $TEST_INPUT: Cadical successfully verified the assignment"
    fi
done <<< "$CNF_FILES"

# Delete tmp.cnf file
rm -f $TMP_CNF $ASS_FILE

# Print the number of failed tests
echo
echo "==================== Summary ===================="
if [ $FAILED -eq 0 ]; then
    echo -e "All tests: ${GREEN}PASSED${NC}"
else
    echo -e "$FAILED/$NUM_TESTS tests ${RED}FAILED${NC}"
    return 1
fi
echo "================================================="
