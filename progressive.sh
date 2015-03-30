#!/bin/bash
# hacky bash script to incrementally run parts of the code to show progress

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 input_file output_basename"
    exit 1
fi
infile="$1"
outname="$2"

# function changes a config parameter in config.h to the given value
function chparam () {
    param="$1"
    val="$2"
    sed -i "s/$1 .*/$1 $2/" config.h
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
mkdir -p imgs/$outname
make
echo ./draw "$infile" "imgs/$outname/${n}.png"
./draw "$infile" "imgs/$outname/${n}.png"
for i in $params; do
    n=$((n+1))
    chparam "$i" "true"
    make
    echo ./draw "$infile" "imgs/$outname/${n}.png"
    ./draw "$infile" "imgs/$outname/${n}.png"
done
n=$((n+1))
chparam "DRAW_POLYGON" "false"
make
echo ./draw "$infile" "imgs/$outname/${n}.png"
./draw "$infile" "imgs/$outname/${n}.png"

# restore old config file
mv config.h.bak config.h
