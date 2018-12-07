#!/bin/bash

function _package
{
    DEST=$1

    make clean
    make --file=Makefile.afs unstrip-static
    mkdir $DEST
    cp unstrip-static $DEST/unstrip
    strip $DEST/unstrip
    cp generate-learn-binary.bash $DEST 
    cp foo.c $DEST
    cp *.db $DEST
    cp README $DEST
    tar czvf $DEST.tgz $DEST
}

DIR=$PWD

if [[ $PLATFORM == "x86_64-unknown-linux2.4" ]]; then
    # Do this on x86_64
    DEST=unstrip-64
    echo "Packaging $DEST"
    _package $DEST
else 
    DEST=unstrip-32
    echo "Packaging $DEST"
    _package $DEST
fi

