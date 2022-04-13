#!/bin/bash

# I copied all doc/ folders in dyninst to latex subdirectory here, 
# ensured paths were realtive (e.g., examples and doc paths were off) and then ran:
cd latex
here=$PWD
for file in $(find . -type f -name \*.tex); do 
  filename=$(basename $file)
  dir=$(dirname $file)
  name="${filename%.*}"
  outfile=$name.rst
  cd $dir
  pandoc $filename -o $outfile
  cd $here
done

for file in $(find . -type f -name \*.rst); do 
  filename=$(basename $file)
  dir=$(dirname $file)
  mkdir -p ../$dir
  cp $file ../$dir/$filename
done
