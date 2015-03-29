#!/bin/bash
# runs ./draw on all input files in datasets/

mkdir -p imgs
cd datasets
for file in *; do
    echo "Running with input '$file':"
    ../draw "$file" ../imgs/"${file}.png"
    echo
done
