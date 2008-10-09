#!/bin/sh

REM_SHELL=${XPLAT_RSH:-"ssh"}

if [ $# -ne 3 ]; then
    echo Usage: $0 be_executable be_host_list be_connection_list
    exit 1
fi

declare -a BE_HOSTS
export BE_HOSTS=( `cat $2` )
NBE=${#BE_HOSTS[*]}

declare -a BE_CONN
export BE_CONN=( `cat $3` )
NC=${#BE_CONN[*]}

if [ $NBE -ne $NC ]; then
    echo Number of backends $NBE from $2 not equal to number of connections $NC from $3
    exit 1
fi

export ITER=0
while [ $ITER -lt $NBE ]; do
    # start BE on each host using parent info in BE_MAP
    $REM_SHELL -n ${BE_HOSTS[$ITER]} $1 `echo ${BE_CONN[$ITER]} | awk -F: '{print $1,$2,$3}'` `expr $ITER + 10000` & 
    ITER=`expr $ITER + 1`
done
