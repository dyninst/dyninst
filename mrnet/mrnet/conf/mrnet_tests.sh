#!/bin/sh

TOPGEN=mrnet_topgen
topology_dir="topology_files"

topologies=(1x1 1x2 1x16 1x1x1 1x2x2 1x16x16 1x1x2 1x1x16 1x2x4 1x4x16)
remote_topology_specs=(1 2 16 1:1 2:2x1 16:16x1 1:2 1:16 2:2x2 4:4x4)
#topologies=(1x1x16 1x2x4 1x4x16)
#remote_topology_specs=(1:16 2:2x2 4:4x4)

print_usage()
{
    echo "Usage:  $cmdname [ -l | -r <hostfile> | -a <hostfile> | -f <sharedobject> ]"
    return 0
}

print_help()
{
    echo "Usage:  $cmdname [ -l | -r <hostfile> | -a <hostfile> | -f <sharedobject>]"
    echo "    -l will run tests on the local host only"
    echo "    -r will run tests on remote hosts and requires a hostfile"
    echo "    -a will run tests locally and remotely and requires a hostfile"
    echo "    -f will additionally test dynamic filter loading and requires a .so file"
    return 0
}

create_remote_topologies()
{
	# $1, 1st arg, is the hostfile mapping localhost:n => remotehost
    remote_topology_prefix="remote"

    for (( idx = 0 ; idx < ${#topologies[@]}; idx++ ));
    do
		remote_topology="$remote_topology_prefix-${topologies[$idx]}.top"

        echo -n "Creating \"$remote_topology\" ... "

        /bin/rm -f $remote_topology

        $TOPGEN --other ${remote_topology_specs[$idx]} $1 $remote_topology

        if [ "$?" == 0 ]; then
            echo "success!"
        else
            echo "failure!"
        fi
    done
}

run_test()
{
    # $1, 1st arg, is front-end program
    # $2, 2nd arg, is back-end program
    # $3, 3rd arg, says to use local or remote topology files
    front_end=$1
    back_end=$2
    test=`basename $front_end`

    for (( idx = 0 ; idx < ${#topologies[@]}; idx++ ));
    do
        topology=${topologies[$idx]}
        if [ $3 = "remote" ]; then
            topology_file="remote-$topology.top"
            outfile="$test-remote-$topology.out"
            logfile="$test-remote-$topology.log"
            echo -n "Running $test(\"remote\", \"$topology\") ... "
        else
            topology_file="$topology_dir/local-$topology.top"
            outfile="$test-local-$topology.out"
            logfile="$test-local-$topology.log"
            echo -n "Running $test(\"local\", \"$topology\") ... "
        fi

        /bin/rm -f $outfile
        /bin/rm -f $logfile

        case "$front_end" in
        "test_DynamicFilters_FE" )
            $front_end $4 $topology_file $back_end > $outfile 2> $logfile
            ;;
        "microbench_FE" )
            $front_end 5 500 $topology_file $back_end > $outfile 2> $logfile
            ;;
        * )
            $front_end $topology_file $back_end > $outfile 2> $logfile
            ;;
        esac
        if [ "$?" = 0 ]; then
            echo "exited with $?."
            echo -n "   Checking results ... ";
            /bin/rm -f $logfile

            grep -i failure $outfile > grep.out
            if [ "$?" != 0 ]; then
                echo "success!"
                /bin/rm -f $logfile
            else
                echo "failure! (Details in $outfile and $logfile)"
            fi
        /bin/rm -f grep.out
        else
            echo "Test exited with failure.(Details in $outfile and $logfile)"
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
            echo "    Must specify a hostfile after -a option"
            exit -1;
        fi
        if test -r $1; then
            hostfile="$1"
        else
            echo "    $1 doesn't exist or is not readable"
            exit -1
        fi
        shift
        ;;
    -r )
        remote="true"
        local="false"
        shift
        if [ $# -lt 1 ]; then
            print_usage
            echo "    Must specify a hostfile after -r option"
            exit -1;
        fi
        if test -r "$1"; then
            hostfile="$1"
        else
            echo "    $1 doesn't exist or is not readable"
            exit -1
        fi
		shift
        ;;
    -f )
        shift
        if [ $# -lt 1 ]; then
            print_usage
            echo "    Must specify a .so file after -f option"
            exit -1;
        fi
        sharedobject="$1"
        shift
        ;;
    *)
      print_usage;
      exit 0;
      ;;
  esac
done

if [ "$local" == "true" ]; then
    run_test "test_basic_FE" "test_basic_BE" "local" ""
    echo
    run_test "test_arrays_FE" "test_arrays_BE" "local" ""
    echo
    run_test "test_NativeFilters_FE" "test_NativeFilters_BE" "local" ""
    echo
    if [ "$sharedobject" != "" ]; then
        run_test "test_DynamicFilters_FE" "test_DynamicFilters_BE" "local" $sharedobject
        echo
    fi
    run_test "microbench_FE" "microbench_BE" "local" ""
    echo
fi

if [ "$remote" == "true" ]; then
	create_remote_topologies $hostfile

    run_test "test_basic_FE" "test_basic_BE" "remote" ""
    echo
    run_test "test_arrays_FE" "test_arrays_BE" "remote" ""
    echo
    run_test "test_NativeFilters_FE" "test_NativeFilters_BE" "remote" ""
    echo
    if [ "$sharedobject" != "" ]; then
        run_test "test_DynamicFilters_FE" "test_DynamicFilters_BE" "remote" $sharedobject
        echo
    fi
    run_test "microbench_FE" "microbench_BE" "remote" ""
    echo
fi
