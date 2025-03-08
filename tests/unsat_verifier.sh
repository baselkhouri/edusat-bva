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

DRATTRIM=$(find $WORKDIR/extern/drat-trim -name "drat-trim" -type f)

# Check if binary was found
if [ -z "$DRATTRIM" ]; then
    echo "Error: 'drat-trim' binary not found."
    return 1
fi

# Store the list of .cnf files in a variable
CNF_FILES=$(find "$WORKDIR/tests/$TEST_FOLDER" -name "*no*.cnf")
PROOF_FILE="proof.drat"

# Check if any files were found
if [[ -z "$CNF_FILES" ]]; then
    echo "Error: No UNSAT .cnf files found in $WORKDIR/tests/$TEST_FOLDER."
    return 1  # Stop execution safely
else
    echo "Found $(echo "$CNF_FILES" | wc -l) UNSAT .cnf files."
fi
# Loop through the files safely
while IFS= read -r TEST_INPUT; do
    OUTPUT=$($BINARY -proof $PROOF_FILE $TEST_INPUT)
    NUM_TESTS=$((NUM_TESTS+1))
    if [[ ! $OUTPUT == *"UNSAT"* ]]; then
        echo -e "[${RED}FAILED${NC}] $TEST_INPUT: Unexpected result. Should be UNSAT."
        FAILED=$((FAILED+1))
        continue
    fi

    # Run drat-trim
    OUTPUT_DRATRIM=$($DRATTRIM $TEST_INPUT $PROOF_FILE)
    if [[ ! $OUTPUT_DRATRIM == *"VERIFIED"* ]]; then
        echo -e "[${RED}FAILED${NC}] $TEST_INPUT: DRAT-trim failed to verify the proof."
        FAILED=$((FAILED+1))
    else
        echo -e "[${GREEN}PASSED${NC}] $TEST_INPUT: DRAT-trim verified the proof."
    fi
done <<< "$CNF_FILES"

# Delete proof.drat file
rm -f $PROOF_FILE

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
