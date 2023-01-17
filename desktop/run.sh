#!/bin/bash
mkdir -p build
cd build
cmake ..
make
if [[ $? -ne 0 ]]; then
    exit 1
fi
./main