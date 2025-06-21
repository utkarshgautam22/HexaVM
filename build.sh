#!/bin/bash

# Compile the vm runtime with all source files
echo "Compiling vm runtime..."
g++ -o vm run.cpp setup.cpp cpu.cpp assembler.cpp -std=c++11

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful!"
    echo "Usage: ./vm <input.asm> [-r] [output.bin]"
    echo "  -r         : Run the program after assembling"
    echo "  output.bin : Save assembled binary to file (optional)"
else
    echo "Compilation failed."
fi
