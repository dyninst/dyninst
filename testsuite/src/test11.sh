#!/bin/sh

GREP="grep -q"
GZIP=gzip
PS=ps
EXPR=expr

FAIL_COUNT=0
SUCCESS_STR='information: the execution of mutatee terminates...'

if [ x$PLATFORM = x ]; then
    echo Environment variable PLATFORM must be set prior to running this test.
    exit
fi

# Find our way to core/codeCoverge
case `basename $PWD` in
    testprog)
	cd ../../..
	BASEDIR=`pwd`/core
	SCRIPTDIR=`pwd`/scripts
	;;
    $PLATFORM)
	cd ../../../..
	BASEDIR=`pwd`/core
	SCRIPTDIR=`pwd`/scripts
	;;
    *)
	if [ x$PARADYN_BASE = x ]; then
	    BASEDIR=$DYNINST_ROOT/core
	    SCRIPTDIR=$DYNINST_ROOT/scripts
	else
	    BASEDIR=$PARADYN_BASE/core
	    SCRIPTDIR=$PARADYN_BASE/scripts
	fi
	;;
esac

if [ ! -d $BASEDIR ]; then
    echo "I cannot find the codeCoverage directory."
    exit
fi

runTest() {
    TESTNUM=$1
    TESTSTR=$2
    shift 2

    ($SCRIPTDIR/timer.pl -t 1200 -r '^\S*ijpeg' -r '^\S*cc1' -r '\S*dyncov' $@ 2>&1) | $GREP "$SUCCESS_STR"
    RETVAL=$?

    if [ -f core ]; then
	echo "**Failed** test #$TESTNUM ($TESTSTR)"
	echo "    - Core file produced."
	FAIL_COUNT=`$EXPR $FAIL_COUNT + 1`
	rm -f core

    elif [ $RETVAL != 0 ]; then
	echo "**Failed** test #$TESTNUM ($TESTSTR)"
	echo "    - Success string not found in output."
	FAIL_COUNT=`$EXPR $FAIL_COUNT + 1`

    else
	echo "Passed test #$TESTNUM ($TESTSTR)"
    fi
}

cd $BASEDIR/codeCoverage/$PLATFORM/tests/ijpeg
DYNCOV_IJPEG_ARGS="--run ./ijpeg
		   -image_file specmun.ppm
		   -compression.quality 90
		   -compression.optimize_coding 0
		   -compression.smoothing_factor 90
		   -difference.image 1
		   -difference.x_stride 10
		   -difference.y_stride 10
		   -verbose 1 -GO.findoptcomp"

# Fake output to simulate standard test
echo "*** dyninstAPI test11..."
echo 
echo "[Tests with ijpeg]"
$GZIP -df ijpeg.gz
echo
echo '"test11 -mutatee test11.mutatee_ijpeg"'
echo
runTest 1 'IJPEG Pre-Instrumentation, All Blocks'			\
	../../dyncov $DYNCOV_IJPEG_ARGS
runTest 2 'IJPEG Pre-Instrumentation, All Blocks, Deletion'		\
	../../dyncov --del 10 $DYNCOV_IJPEG_ARGS
runTest 3 'IJPEG Pre-Instrumentation, Dominator'			\
	../../dyncov --dom $DYNCOV_IJPEG_ARGS
runTest 4 'IJPEG Pre-Instrumentation, Dominator, Deletion'		\
	../../dyncov --dom --del 10 $DYNCOV_IJPEG_ARGS
runTest 5 'IJPEG On-Demand Instrumentation, All Blocks'			\
	../../dyncov --ond $DYNCOV_IJPEG_ARGS
runTest 6 'IJPEG On-Demand Instrumentation, All Blocks, Deletion'	\
	../../dyncov --ond --del 10 $DYNCOV_IJPEG_ARGS
runTest 7 'IJPEG On-Demand Instrumentation, Dominator'			\
	../../dyncov --ond --dom $DYNCOV_IJPEG_ARGS
runTest 8 'IJPEG On-Demand Instrumentation, Dominator, Deletion'	\
	../../dyncov --ond --dom --del 10 $DYNCOV_IJPEG_ARGS

echo
if [ $FAIL_COUNT = 0 ]; then
    echo "All tests passed"

elif [ $FAIL_COUNT = 1 ]; then
    echo "**Failed** $FAIL_COUNT test"

else
    echo "**Failed** $FAIL_COUNT tests"
fi

FAIL_COUNT=0

cd $BASEDIR/codeCoverage/$PLATFORM/tests/cc1
DYNCOV_CC1_ARGS="--run ./cc1 `ls *.i`"

# Fake output to simulate standard test
echo
echo "[Tests with cc1]"
$GZIP -df cc1.gz
echo
echo '"test11 -mutatee test11.mutatee_cc1"'
echo
runTest 1 'cc1 Pre-Instrumentation, All Blocks'				\
	../../dyncov $DYNCOV_CC1_ARGS
runTest 2 'cc1 Pre-Instrumentation, All Blocks, Deletion'		\
	../../dyncov --del 10 $DYNCOV_CC1_ARGS
runTest 3 'cc1 Pre-Instrumentation, Dominator'				\
	../../dyncov --dom $DYNCOV_CC1_ARGS
runTest 4 'cc1 Pre-Instrumentation, Dominator, Deletion'		\
	../../dyncov --dom --del 10 $DYNCOV_CC1_ARGS
runTest 5 'cc1 On-Demand Instrumentation, All Blocks'			\
	../../dyncov --ond $DYNCOV_CC1_ARGS
runTest 6 'cc1 On-Demand Instrumentation, All Blocks, Deletion'		\
	../../dyncov --ond --del 10 $DYNCOV_CC1_ARGS
runTest 7 'cc1 On-Demand Instrumentation, Dominator'			\
	../../dyncov --ond --dom $DYNCOV_CC1_ARGS
runTest 8 'cc1 On-Demand Instrumentation, Dominator, Deletion'		\
	../../dyncov --ond --dom --del 10 $DYNCOV_CC1_ARGS

echo
if [ $FAIL_COUNT = 0 ]; then
    echo "All tests passed"

elif [ $FAIL_COUNT = 1 ]; then
    echo "**Failed** $FAIL_COUNT test"

else
    echo "**Failed** $FAIL_COUNT tests"
fi
