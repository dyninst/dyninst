#!/bin/sh

if [ $# -lt 1 ]; then
  echo "Usage: stash_tickets.sh $hostfile"
  exit -1;
fi

hosts=`cat $1`

for host in $hosts
do
	echo -n "Stashing ticket on $host ..."

	ssh $host /s/std/bin/stashticket

	if [ "$?" != 0 ]; then
        echo "failure!"
    else
        echo "success!"
    fi
done
