#!/bin/bash
# runs ./draw on all input files in datasets/

mkdir -p imgs
cd datasets
for file in *; do
    ../draw "$file" ../imgs/"${file}.png"
done
