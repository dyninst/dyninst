#!/bin/sh

SYSNAME=$1

case $SYSNAME in

i[3456]86-unknown-linux)
    echo "i386-unknown-linux2.4"
    ;;

ia64-unknown-linux)
    echo "ia64-unknown-linux2.4"
    ;;

x86_64-unknown-linux)
    echo "x86_64-unknown-linux2.4"
    ;;

esac
