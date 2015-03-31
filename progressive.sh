#!/bin/bash -e
# hacky bash script to incrementally run parts of the code to show progress
# should only be used on comb_files with one comb that has one set

if [ "$#" -lt 3 ]; then
    echo "Usage: $0 point_file comb_file output_basename"
    exit 1
fi
pointfile="$1"
combfile="$2"
outname="$3"

# function changes a config parameter in config.h to the given value
function chparam () {
    param="$1"
    val="$2"
    sed -i "s/$1 .*/$1 $2/" config.h
}

# function to run with the current config file
function runcode () {
    n="$1"
    make
    mkdir -p "progout/$outname"
    echo ./draw "$pointfile" "$combfile" "progout/$outname/foo"
    ./draw "$pointfile" "$combfile" "progout/$outname/foo"
    mv "progout/$outname/foo_0_0.png" "progout/$outname/$n.png"
}

# ordered list of parameters to change
params="DRAW_POLYGON RUN_FIX_HULL RUN_REFINE_POLY RUN_RM_CROSSING DRAW_BLOB"

# backs up config.h
cp config.h config.h.bak

# set all params to false initially
for i in $params; do 
    chparam $i "false"
done

# incrementally generate images
n=1
runcode "$n"
for i in $params; do
    n=$((n+1))
    chparam "$i" "true"
    runcode "$n"
done
n=$((n+1))
chparam "DRAW_POLYGON" "false"
runcode "$n"

# restore old config file
mv config.h.bak config.h
