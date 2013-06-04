#!/bin/bash

BASE_VERSION=v8.1.1
 
for dir in . dyninstAPI symtabAPI instructionAPI parseAPI dataflowAPI stackwalk patchAPI proccontrol dynC_API
do
    cd $DYNINST_ROOT/dyninst/$dir
    git log --no-merges --no-decorate --oneline $BASE_VERSION..HEAD $DYNINST_ROOT/dyninst/$dir > CHANGELOG
done
