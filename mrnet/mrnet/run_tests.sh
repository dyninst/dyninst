#!/bin/sh

bindir="/p/paradyn/development/darnold/paradyn/core/mrnet/mrnet/bin/i686-pc-linux-gnu/"
topology_dir="/p/paradyn/development/darnold/paradyn/core/mrnet/mrnet/tests/topology_files"
topologies="1x1 1x2 1x16 1x1x1 1x2x2 1x16x16 1x1x2 1x1x16 1x2x4 1x4x16"

print_usage()
{
    echo "Usage:  $cmdname [ -l | -r <hostfile> | -a <hostfile> | -f <sharedobject> ]"
    return 0
}

print_help()
{
    echo "Usage:  $cmdname [ -l | -r <hostfile> | -a <hostfile> | -f <sharedobject>]"
    echo "\t-l will run tests on the local host only"
    echo "\t-r will run tests on remote hosts and requires a hostfile"
    echo "\t-a will run tests locally and remotely and requires a hostfile"
    echo "\t-f will additionally test dynamic filter loading and requires a .so file"
    return 0
}

test_basic()
{
    # $1, 1st arg, says to use local or remote topology files
    if [ $1 = "remote" ]; then
        topology_prefix="remote"
    else
        topology_prefix="local"
    fi

    front_end="$bindir/test_basic_FE"
    back_end="$bindir/test_basic_BE"

    for file in $topologies
    do
        echo -n "Running test_basic(\"$topology_prefix\", \"$file\") ... "
        outfile="basic-$topology_prefix-$file.out"
        /bin/rm -f $outfile

        topology="$topology_dir/$topology_prefix-$file.top"
        $front_end $topology $back_end > $outfile 2>&1

        grep -i failure $outfile > grep.out
        if [ "$?" != 0 ]; then
            echo "success!"
        else
            echo "failure! (Details in $outfile)"
        fi
    done
}

test_arrays()
{
    # $1, 1st arg, says to use local or remote topology files
    if [ $1 = "remote" ]; then
        topology_prefix="remote"
    else
        topology_prefix="local"
    fi

    front_end="$bindir/test_arrays_FE"
    back_end="$bindir/test_arrays_BE"

    for file in $topologies
    do
        echo -n "Running test_arrays(\"$topology_prefix\", \"$file\") ... "
        outfile="arrays-$topology_prefix-$file.out"
        /bin/rm -f $outfile

        topology="$topology_dir/$topology_prefix-$file.top"
        $front_end $topology $back_end > $outfile 2>&1

        grep -i failure $outfile > grep.out
        if [ "$?" != 0 ]; then
            echo "success!"
        else
            echo "failure! (Details in $outfile)"
        fi
    done
}

test_native_filters()
{
    # $1, 1st arg, says to use local or remote topology files
    if [ $1 = "remote" ]; then
        topology_prefix="remote"
    else
        topology_prefix="local"
    fi

    front_end="$bindir/test_NativeFilters_FE"
    back_end="$bindir/test_NativeFilters_BE"

    for file in $topologies
    do
        echo -n "Running test_native_filters(\"$topology_prefix\", \"$file\") ... "
        outfile="native_filters-$topology_prefix-$file.out"
        /bin/rm -f $outfile

        topology="$topology_dir/$topology_prefix-$file.top"
        $front_end $topology $back_end > $outfile 2>&1

        grep -i failure $outfile > grep.out
        if [ "$?" != 0 ]; then
            echo "success!"
        else
            echo "failure! (Details in $outfile)"
        fi
    done
}

test_dynamic_filters()
{
    # $1, 1st arg, says to use local or remote topology files
    if [ "$1" == "remote" ]; then
        topology_prefix="remote"
    else
        topology_prefix="local"
    fi

    front_end="$bindir/test_DynamicFilters_FE"
    back_end="$bindir/test_DynamicFilters_BE"

    for file in $topologies
    do
        echo -n "Running test_dynamic_filters(\"$topology_prefix\", \"$file\") ... "
        outfile="dynamic_filters-$topology_prefix-$file.out"
        /bin/rm -f $outfile

        topology="$topology_dir/$topology_prefix-$file.top"
        $front_end $sharedobject $topology $back_end > $outfile 2>&1

        grep -i failure $outfile > grep.out
        if [ "$?" != 0 ]; then
            echo "success!"
        else
            echo "failure! (Details in $outfile)"
        fi
    done
}

microbench()
{
    # $1, 1st arg, says to use local or remote topology files
    if [ $1 = "remote" ]; then
        topology_prefix="remote"
    else
        topology_prefix="local"
    fi

    front_end="$bindir/microbench_FE"
    back_end="$bindir/microbench_BE"

    for file in $topologies
    do
        echo -n "Running microbench(\"$topology_prefix\", \"$file\") ... "
        outfile="microbench-$topology_prefix-$file.out"
        /bin/rm -f $outfile

        topology="$topology_dir/$topology_prefix-$file.top"
        $front_end 5 500 $topology $back_end > $outfile 2>&1

        grep -i failure $outfile > grep.out
        if [ "$?" != 0 ]; then
            echo "success!"
        else
            echo "failure! (Details in $outfile)"
        fi
    done
}

##### "main()" starts here
## Parse the command line

if [ $# -lt 1 ]; then
  print_usage
  exit -1;
fi

local="true"
remote="false"
sharedobject=""

while [ $# -gt 0 ]
do
  case $1 in
    -l )
        local="true"
        shift
        ;;
    -a )
        local="true"
        remote="true"
        shift
        if [ $# -lt 1 ]; then
            print_usage
            echo "\tMust specify a hostfile after -a option"
            exit -1;
        fi
        if test -r $1; then
            hostfile="$1"
        else
            echo "$1 doesn't exist or is not readable"
            exit -1
        fi
        ;;
    -r )
        remote="true"
        local="false"
        shift
        if [ $# -lt 1 ]; then
            print_usage
            echo "\tMust specify a hostfile after -r option"
            exit -1;
        fi
        if test -r $1; then
            hostfile="$1"
        else
            echo "$1 doesn't exist or is not readable"
            exit -1
        fi
        ;;
    -f )
        shift
        if [ $# -lt 1 ]; then
            print_usage
            echo "\tMust specify a .so file after -f option"
            exit -1;
        fi
        if test -r $1; then
            sharedobject="$1"
        else
            echo "$1 doesn't exist or is not readable"
            exit -1
        fi
        shift
        ;;
    *)
      print_usage;
      exit 0;
      ;;
  esac
done

if [ "$local" == "true" ]; then
    test_basic "local"
    echo
    test_arrays "local"
    echo
    test_native_filters "local"
    echo
    if [ "$sharedobject" != "" ]; then
        test_dynamic_filters "local" $sharedobject
        echo
    fi
    microbench "local"
    echo
fi

if [ "$remote" == "true" ]; then
    #test_basic "remote"
    echo
    #test_arrays "remote"
    echo
    #test_native_filters "remote"
    echo
    if [ "$sharedobject" != "" ]; then
        #test_dynamic_filters "remote" $sharedobject
        echo
    fi
    #microbench "remote" $hostfile
    echo
fi
