#!/bin/bash

touch input

for i in {1..20000}
do
    echo 08000000 >> input
    echo 18000000 >> input
    echo 97ffffd6 >> input
    echo 17ffffdb >> input
    echo 0	  >> input
done

echo -1 >> input
