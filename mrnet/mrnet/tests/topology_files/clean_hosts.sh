#!/bin/sh

if [ $# -lt 1 ]; then
  echo "Usage: clean_hosts.sh $hostfile"
  exit -1;
fi

hosts=`cat $1`

for host in $hosts
do
	echo -n "Cleaning processes on $host ..."

	ssh $host ps x ; killall -9 ssh

	if [ "$?" != 0 ]; then
        echo "failure!"
    else
        echo "success!"
    fi
done
