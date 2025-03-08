#!/bin/bash

# Get the directory of the current script
WORKDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Create an alias to the binary file
EDUSAT="$WORKDIR/build/edusat"
echo "Creating alias to the binary..."
alias edusat="$EDUSAT"
echo "Alias 'edusat' created successfully!"

# Create an alias to the cadical binary file
CADICAL="$WORKDIR/extern/cadical/build/cadical"
alias cadical="$CADICAL"
echo "Alias 'cadical' created successfully!"
