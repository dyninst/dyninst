#!/bin/tcsh

set index = 0
if($# != 1) then
	echo "usage: " $0 " [ijpeg|cc1]"
	exit
endif


set SHELL_TCSH = /bin/tcsh

set ARG = ''

if($1 == "cc1") then
	set ARG = '--run ./cc1 `ls *.i`'
else if($1 == "ijpeg") then
	set ARG = '--run ./ijpeg -image_file specmun.ppm -compression.quality 90 -compression.optimize_coding 0 -compression.smoothing_factor 90 -difference.image 1 -difference.x_stride 10 -difference.y_stride 10 -verbose 1 -GO.findoptcomp'
else
	echo "usage: " $0 " [ijpeg|cc1]"
	exit
endif

cd $DYNINST_ROOT/core/codeCoverage/$PLATFORM/tests/$1

set BACK = "&"
set EXECPATH = ../..

set FILE = "$1.runs.sh"

rm -rf $FILE
rm -rf *.out

touch $FILE
chmod a+x $FILE
echo "#\!$SHELL_TCSH" >> $FILE

foreach i ( "" "--ond" )
	foreach j ( "" "--dom")
		foreach k ( "" "--del 10")
			touch $index.out
			echo "" >> $index.out
			echo "" >> $index.out
			echo "**************** [$i $j $k] **************" >> $index.out
			echo "" >> $index.out
			echo "" >> $index.out


			echo "$EXECPATH/dyncov $i $j $k --suffix .dyncov$index $ARG >>&! $index.out $BACK" >> $FILE

			@ index++

			if($index == 4) then
				set BACK = ""
			endif
			if($index == 5) then
				set BACK = "&"
			endif
		end
	end
end

echo ""
echo ""
echo ""
echo "To run the code coverage test on $1"
echo ""
echo "	1. First change the directory on ./$1"
echo "	2. Run $FILE script"
echo "		(First please check whether tcsh path is correct in $FILE)"
echo "		(The default is to run first 5 runs at the same time"
echo "		If you want to run tests in sequential order (1 processor)"
echo "		please modify the $FILE accordingly)"
echo "	3. Check the [0-9].out text files whether application terminated successfully"
echo "	4. Check whether cc1.dyncov[0-9] binary files are generated"
echo ""
echo ""
