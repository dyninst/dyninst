#!/bin/sh

RM=/bin/rm
MV=/bin/mv

cmdname=`basename $0`

print_usage()
{
  echo "Usage:  $cmdname src_file obj_file dep_file [inc_dir]"
    return 0
}

if [ $# -gt 3 ]; then
  src_file=$1
  obj_file=$2
  dep_file=$3
else
  print_usage;
  exit 0;
fi

shift
shift
shift

while [ $# -gt 0 ]
do
  if [ $1 != "-I" ]; then
    tmp=`echo $1 | sed 's/-I//'`
    incdirs="$incdirs $tmp"
  fi
  shift
done

grep_output=`grep "#include *\"" $src_file`

for word in $grep_output
do
  if [ $word != "#include" ]; then
    file=`echo $word | tr -d \"`
    for dir in $incdirs
    do
      if [ -e $dir/$file ]; then
        inc_files="$inc_files $dir/$file"
        break
      fi
    done
  fi
done

$RM -f $dep_file.tmp
echo "$obj_file: \\" > $dep_file.tmp
for i in  $inc_files
do
  echo "  $i \\" >> $dep_file.tmp
done
echo >> $dep_file.tmp

if !diff $dep_file.tmp $dep_file >& /dev/null; then
    $MV -f $dep_file.tmp $dep_file
else
    $RM $dep_file.tmp
fi ;
