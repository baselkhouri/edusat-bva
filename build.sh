#!/bin/bash

# Ensure we're in the root of the project
WORKDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Build Edusat
echo "Building Edusat..."
make -C "$WORKDIR"

# Build CaDiCaL only if needed
if [ ! -f "$CADICAL" ]; then
    echo "Building CaDiCaL..."
    cd "$WORKDIR/extern/cadical"
    ./configure && make -j40
    cd "$WORKDIR"
else
    echo "CaDiCaL is already built."
fi

# Build Drat-trim
echo "Building drat-trim..."
cd "$WORKDIR/extern/drat-trim"
make
cd "$WORKDIR"
