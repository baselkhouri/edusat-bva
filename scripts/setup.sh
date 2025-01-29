#!/bin/bash

# Ensure we're in the root of the project
PROJECT_DIR=$(pwd)

# Run 'make clean' to clean the build directory
echo "Cleaning build directory..."
make clean

# Run 'make' to build the project
echo "Building the project..."
make

# Create an alias to the binary file
BINARY="$PROJECT_DIR/build/edusat"
if [ -f "$BINARY" ]; then
    echo "Creating alias to the binary..."
    alias edusat="$BINARY"
    echo "Alias 'edusat' created successfully!"
else
    echo "Error: Binary 'edusat' not found after build."
fi

echo "Setup complete."
