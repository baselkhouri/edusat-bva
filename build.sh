#!/bin/bash

# Ensure we're in the root of the project
WORKDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Build Edusat
echo "Building Edusat..."
make -C "$WORKDIR"

# Set executable paths
EDUSAT="$WORKDIR/build/edusat"
CADICAL="$WORKDIR/extern/cadical/build/cadical"

# Export executables (instead of aliases)
echo "Setting up environment variables..."
alias edusat="$EDUSAT"
alias cadical="$CADICAL"

# Build CaDiCaL only if needed
if [ ! -f "$CADICAL" ]; then
    echo "Building CaDiCaL..."
    cd "$WORKDIR/extern/cadical"
    ./configure && make
    cd "$WORKDIR"
else
    echo "CaDiCaL is already built."
fi