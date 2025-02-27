#!/bin/bash
# Run this from tests root directory

TEST_ROOT=$(PWD)

# Define color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Counters
FAILED=0
NUM_TESTS=0

# Find edusat binary in ../build directory
BINARY=$(find ../build -name "edusat")
# Check if binary was found
if [ -z "$BINARY" ]; then
    echo "Error: 'edusat' binary not found. Try running 'make' in the project root directory."
else
    # Run each test input with the binary in "easy_cnf_instances" directory
    # Take the output of the run, and check if contains UNSAT or SAT
    # If it contains SAT, check if the file name has "yes" int it
    # If it contains UNSAT, check if the file name has "no" in it
    # If the output is not as expected, print an error message for the specific test
    # TODO: Later on, include more tests if added
    for TEST_INPUT in $(find easy_cnf_instances -name "*.cnf"); do
        OUTPUT=$($BINARY $TEST_INPUT)
        NUM_TESTS=$((NUM_TESTS+1))
        if [[ $OUTPUT == *"UNSAT"* ]]; then
            if [[ $TEST_INPUT == *"yes"* ]]; then
                echo -e "[${RED}FAILED${NC}] $TEST_INPUT: Expected 'SAT', got 'UNSAT'."
                FAILED=$((FAILED+1))
            else
                # echo -e "[${GREEN}PASSED${NC}] $TEST_INPUT: Test passed."
            fi
        elif [[ $OUTPUT == *"SAT"* ]]; then
            if [[ $TEST_INPUT == *"no"* ]]; then
                echo -e "[${RED}FAILED${NC}] $TEST_INPUT: Expected 'UNSAT', got 'SAT'."
                FAILED=$((FAILED+1))
            else
                # echo -e "[${GREEN}PASSED${NC}] $TEST_INPUT: Test passed."
            fi
        else
            echo -e "[${RED}FAILED${NC}] $TEST_INPUT: Expected 'SAT' or 'UNSAT', got '$OUTPUT'."
        fi
    done

    # Delete assignment.txt file if it exists
    rm -f assignment.txt

    # Print the number of failed tests
    echo
    echo "==================== Summary ===================="
    if [ $FAILED -eq 0 ]; then
        echo -e "All tests: ${GREEN}PASSED${NC}"
    else
        echo -e "$FAILED/$NUM_TESTS tests ${RED}FAILED${NC}"
    fi
    echo "================================================="
fi