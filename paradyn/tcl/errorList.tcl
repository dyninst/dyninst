#
# $Log: errorList.tcl,v $
# Revision 1.25  1996/04/29 03:29:53  tamches
# Removed newlines from description of error msgs
#
# Revision 1.24  1996/04/04 21:56:40  newhall
# added error 97
#
# Revision 1.23  1996/04/03  14:29:20  naim
# Adding error number 96 - naim
#
# Revision 1.22  1996/03/22  22:03:29  tamches
# added dynRPC version mismatch as possible explanations to errors 14 and 84.
#
# Revision 1.21  1996/03/20 17:04:36  mjrg
# Changed mdl to support calls with multiple arguments.
#
# Revision 1.20  1996/03/12 20:41:19  mjrg
# Improved handling of process termination.
# New version of aggregateSample to support adding and removing components
# dynamically.
# Added error messages.
#

#
# Error message format:
# set pdError(1) { -> Error number 
#
# { Default error message -> Error message to be displayed when "" is received
#			     as a parameter. Detailed information such as
# 			     specific filenames will be displayed only if this
#			     information was included as part of the error
#			     message
# } {source -> Indicates where the error comes from. "source" can be:
#	       paradynd = paradyn daemon
#	       pc = performance consultant
#	       dm = data manager
#	       vi = visi interface 
#	       vm = visi manager
#	       ui = user interface
# } {error type -> information, serious, fatal
# } {
# Explanation -> Detailed explanation of the error and possible actions to be
#		 taken by the user
# }}
#
# Call example: 
# 	showErrorCallback(27, "Executable file /p/paradyn/weird not found");
#

# Note that the default explanations should generally _not_ have a newline in them.
# The tk text widget will word wrap just fine on its own.  Putting in newlines forces
# a newline at certain points in the text widget---and who's to say that it will be in
# the right place since the user can resize the error dialog box to different widths?

set pdError(1) {
{Application Process found for machine without paradynd
} {paradynd
} {serious error
} {
An application process was found to be running on a machine that had no paradynd process running.  This is a serious error that indicates either a paradynd process could not be started, or that it might have died.  This error should be considered an indication of a bug in the tool.
}} 

set pdError(2) {
{Data for unknown metric id}
{dm}
{serious error}
{Data has arrived from a paradynd process for an unknown metric id.  This is a serious error that indicates a bug in the paradyn/paradynd interface.}
} 

set pdError(3) {
{Unable to find metric component for sample.}
{dm}
{serious error}
{A sample value has arrive for a metric from a paradynd, but the paradyn process was not expecting a value from this process.  This is a serious internal consistency failure of the paradyn/paradynd interface.}
} 

set pdError(4) {
{Unable to connect to new paradyn daemon process.}
{paradynd}
{serious error}
{A request had arrived to start a new paradyn daemon process on a remote machine (either from the user or the system based on adding new hosts), and the paradyn user process was unable to rendezvous with the paradynd process.  This could indicate a network failure, the paradynd process not being installed on the remote machine, or a file permission problem.}
} 

set pdError(5) {
{paradynd process has died}
{paradynd}
{information}
{A paradynd process has died somewhere in the system.  This indicates either a network failure, a host failure, or a bug in the paradynd process.}
} 

set pdError(6) {
{Unable to start paradynd}
{dm}
{information}
{A request to start a new application process on a machine required that a new paradyn daemon process be started on that machine.  The attempt to start that new paradynd process failed.  This is a continuable error, but does mean that the request application process will NOT be started.  This error can happen if the path for the paradynd is incorrect, if the paradynd binary is not installed on that machine, or if the machine or network is down.}
} 

set pdError(7) {
{Auto refinement already enabled}
{pc}
{serious error}
{An attempt to enable automatic refinement was made will automated refinement was already being attempted.}
} 

set pdError(8) {
{Unable to find search history graph node}
{pc}
{information}
{An attempt to lookup a search history graph node failed.  The passed integer name of the node was not found in the list of nodes.}
} 

set pdError(9) {
{Search history graph ancestor not true}
{pc}
{information}
{An attempt to set the current refinement to a node failed because one of the ancestors of that node is false.  To manually select a SHG node, you must select a node which is true.  In addition, all of its ancestors back to the root must also be true.}
} 

set pdError(10) {
{malloc failure}
{dm}
{fatal error}
{Call to malloc failed within a data manager function.}
} 

set pdError(11) {
{Application process has exited}
{paradynd}
{information}
{An application process has exited. This situation may be produced, for example, by an unsuccessful request of memory made by this process, or it could be possible that the application just finished. }
} 

set pdError(12) {
{malloc failed in VISIthreadchooseMetRes}
{vi}
{serious error}
{Call to malloc failed within a visi-thread function.}
} 

set pdError(13) {
{thr_getspecific failed}
{vi}
{serious error}
{Call to thr_getspecific in a visi-thread function failed.}
} 

set pdError(14) {
{Unable to start visualization process}
{vi}
{serious error}
{A request to start a new visualization process has failed. Some possible
explanations are:
(1) the executable for this visi is not well installed, and you should check whether the executable is in the right place,
(2) the process you just started is not a visi process, or
(3) Paradyn and the visi process are communicating in an incompatible fasion; e.g. mixing release 1.0 of Paradyn and 1.1 of the visi}
} 

set pdError(15) {
{Unable to create performance stream}
{vi}
{serious error}
{An attempt to create a performance stream for a new visualization failed.}
} 

set pdError(16) {
{Internal error}
{vi}
{serious error}
{Possible causes: bufferSize out of range in VISIthreadDataCallback and remove() in VISIthreadmain. Please, report this error to paradyn@cs.wisc.edu}
} 

set pdError(17) {
{Adding new metrics and/or foci failed}
{vi}
{information}
{An incomplete or invalid metric or focus list was returned as a result of
attempting to add metrics and/or foci to a visualization.}
} 

set pdError(18) {
{malloc failure in visi manager}
{vm}
{fatal error}
{Call to malloc failed within a visi manager function.}
} 

set pdError(19) {
{strdup failure}
{vm}
{fatal error}
{Call to strdup failed within a visi manager function.}
} 

set pdError(20) {
{Internal error}
{vm}
{fatal error}
{An unrecoverable error occurred within a visi manager function. Please, report this error to paradyn@cs.wisc.edu}
} 

set pdError(21) {
{Tcl Command Failure}
{ui}
{fatal error}
{The tcl interpreter has failed. Bad pointer "newptr". Please, report this error to paradyn@cs.wisc.edu}
} 

set pdError(22) {
{Tcl Command Failure}
{ui}
{fatal error}
{The tcl interpreter has failed unexpectedly (getMetsAndRes in UIM::chooseMetricsandResources). Please, report this error to paradyn@cs.wisc.edu }
} 

set pdError(23) {
{Read error}
{paradynd }
{information}
{Read error in application process.}
}

set pdError(24) {
{Unable to read tcl start-up script}
{ui}
{information}
{A tcl error occurred finding or reading the tcl script specified on the paradyn command line with the -s option.}
} 

set pdError(25) {
{Unable to define specified process}
{ui}
{information}
{An error occurred while attempting to define an application.}
} 

set pdError(26) {
{tcl initialization for new resource display object failed.}
{ui}
{fatal error}
{Call to tcl command initRDO failed.}
} 

set pdError(27) {
{Executable not found.}
{paradynd}
{information}
{The executable you are trying to run does not exist. Check out your filename and path again!}
}

set pdError(28) {
{Unable to find symbol.}
{paradynd}
{information}
{Sorry, no more information available.}
}

set pdError(29) {
{Function has bad address.}
{paradynd}
{information}
{Sorry, no more information available.}
}

set pdError(30) {
{Incorrect version number.}
{paradynd}
{information}
{Sorry, no more information available.}
}

set pdError(31) {
{Internal symbol DYNINSTfirst not found.}
{paradynd}
{serious error}
{You have not properly linked your application with the paradyn dyninst library.  Please, refer to the manual pages in order to check how to do this.}
}

set pdError(32) {
{Internal symbol DYNINSTend not found.}
{paradynd}
{serious error}
{This is an internal error. Please, report it to paradyn@cs.wisc.edu}
}

set pdError(33) {
{Could not find version number in instrumentation.}
{paradynd}
{information}
{Your program might has been linked with the wrong version of the paradyn dyninst library, or it could be a non executable binary file.}
}

set pdError(34) {
{Error function without module.}
{paradynd}
{information}
{Sorry, no more information available.}
}

set pdError(35) {
{Unable to open PIF file.}
{paradynd}
{information}
{Sorry, no more information available.}
}

#set pdError(36) {
#{Internal error: non-aligned length received on traceStream.}
#{paradynd}
#{serious error}
#{Please, report this error to paradyn@cs.wisc.edu}
#}

set pdError(36) {
{Received invalid data on trace stream.}
{paradynd}
{serious error}
{Paradyn received invalid data from a process it was tracing. 
There may be many different causes to this problem:
1. a bug in the application being traced;
2. a bug in Paradyn DYNINST library;
3. a bug in the instrumentation code inserted into the 
application;
4. a bug in the paradyn daemon.
Please report this error to paradyn@cs.wisc.edu
}
}

set pdError(37) {
{Internal error: wrong record type on sid}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(38) {
{Error in forwarding signal}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(39) {
{Internal error: unknown process state }
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(40) {
{Internal error: unable to detach PID}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(41) {
{Unable to open file.}
{paradynd}
{information}
{Sorry, no more information available.}
}

set pdError(42) {
{Internal error: unable to parse executable.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(43) {
{Internal error: unable to get loader info about process. }
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(44) {
{Internal error: error reading header}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(45) {
{Internal error: problem with executable header file.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(46) {
{Program not statically linked.}
{paradynd}
{information}
{Sorry, no more information available.}
}

set pdError(47) {
{Dump core failed.}
{paradynd}
{information}
{A paradyn daemon could not dump the core image of a process. This problem can happen because paradyn could not open the file, or it could not write to the file.}
}

set pdError(48) {
{Symbol table out of order, use -Xlinker -bnoobjreorder}
{paradynd}
{information}
{Sorry, no more information available.}
}

set pdError(49) {
{Error reading executable file.}
{paradynd}
{information}
{Sorry, no more information available.}
}

set pdError(50) {
{Internal error: Cannot find file in inst-power.C}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(51) {
{Internal error: In forkNodeProcesses, parent id unknown.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(52) {
{Internal error: Branch too far.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(53) {
{Internal error: Program text + data is too big for dyninst.}
{paradynd}
{fatal error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(54) {
{Warning: Program text + data could be too big for dyninst.}
{paradynd}
{information}
{Sorry, no more information available.}
}

set pdError(55) {
{Internal error: Unsupported return.}
{paradynd}
{fatal error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(56) {
{Internal error: exec failed in paradynd to start paradyndCM5.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(57) {
{Internal error: could not write all bytes.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(58) {
{Internal error: unable to find process.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(59) {
{Internal error: there are no processes known to this daemon.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(60) {
{Internal error: unable to find thread.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(61) {
{Internal error: disableDataCollection mid not found.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(62) {
{Internal error: cannot continue PID.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(63) {
{Internal error: cannot pause PID.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(64) {
{No function calls in procedure.}
{paradynd}
{information}
{No function calls where found in current procedure.}
}

set pdError(65) {
{Sample not for valid metric instance.}
{paradynd}
{information}
{Sorry, no more information available.}
}

set pdError(66) {
{Internal error: inferior heap overflow.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(67) {
{Internal error: attempt to free already freed heap entry.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(68) {
{Internal error: unable to start file.}
{paradynd}
{information}
{Sorry, no more information available.}
}

set pdError(69) {
{Internal error: ptrace error.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(70) {
{Internal error: execv failed. }
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(71) {
{Internal error: vfork failed.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(72) {
{Internal error: unable to stat.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(73) {
{Internal error: could not (un)marshall parameters, dumping core.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(74) {
{Internal error: protocol verification failed.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(75) {
{Internal error: cannot do sync call.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(76) {
{Internal error: unknown message tag. }
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(77) {
{Internal error: handle error called for wrong err_state.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(78) {
{Internal error: problem stopping process.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(79) {
{Internal error: unable to find addr of DYNINSTobsCostLow.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(80) {
{Internal error: unable to find addr of callee process.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(81) {
{Cannot start process on specified host.}
{dm}
{information}
{This error maybe produced by a wrong host name. Paradyn cannot create the
process on the host you are specifying.}
}

set pdError(82) {
{Trying to run a thread that is not ready yet.}
{dm}
{information}
{You are trying to run a process that it is still being created. Please, wait
and try again.}
}

set pdError(83) {
{Internal error: Tcl interpreter failed in routine changeApplicState.}
{ui}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(84) {
{Cannot create new paradyn daemon process.}
{dm}
{serious error}
{An error was detected when a paradyn daemon was being created. Possible explanations for this problem are:
(1) unknown host,
(2) it is not possible to establish connection with the specified host, or
(3) incompatible versions of Paradyn and Paradynd are being used (e.g. release 1.0 of one and release 1.1 of the other).
}
}

set pdError(85) {
{Error found in metrics specified in the Paradyn configuration file.}
{pdMain}
{information}
{An error was detected when Paradyn was reading the metrics described in 
the Paradyn configuration file.}
}

set pdError(86) {
{Cannot enable metric}
{dm}
{information}
{Paradyn cannot enable this particular metric. This might be due to constraints in the definition of the metric (e.g. the metric is restricted to the whole program and we have selected a particular process).}
}

set pdError(87) {
{Error in external visualization process. }
{vi}
{information}
{An error has occurred during the execution of an external visualization process.}
}

set pdError(88) {
{Must define an application before starting the Performance Consultant.}
{ui}
{information}
{No additional information available.}
}

set pdError(89) {
{Command line is missing.}
{vi}
{information}
{The command line is missing from your customized PCL file. You cannot start a process without specifing a command line.}
}

set pdError(90) {
{The Paradyn daemon you have specified does not exist.}
{vi}
{information}
{The Paradyn daemon you have specified does not exist. Paradyn cannot start without a valid Paradyn daemon. Please, try again.}
}

set pdError(91) {
{Paradyn daemon is missing in PCL file.}
{vi}
{information}
{The Paradyn daemon is missing from your customized PCL file. You need to specify a valid Paradyn daemon in order to start an application on a particular machine.}
}

## MDL errors
set pdError(92) {
{Error while evaluating metric.}
{paradynd}
{information}
{A error was found while evaluating a metric for a focus.  The metric cannot be enabled for this focus.
}
} 

set pdError(93) {
{Metric has no constraint that matches focus.}
{paradynd}
{information}
{You tried to enable a metric for a focus, but the metric has no constraint for this focus. Check your focus selection from the where axis.}
}

set pdError(94) {
{Too many arguments to function call in instrumentation code.}
{paradynd}
{serious error}
{An instrumentation request includes a function call with too many arguments.  The maximum number of arguments that can be passed to a function is platform dependent.
}
}

set pdError(95) {
{Unable to find symbol for metric definition.}
{paradynd}
{information}
{Paradyn could not enable a metric because the metric definition has a function call or tries to read the value of a symbol with readSymbol, but paradyn could not find the symbol in the application symbol tables.
}
}

set pdError(96) {
{Internal error: attempt to free non-defined heap entry.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(97) {
{Maximum number of metric/focus pairs have been enabled by this client}
{dm}
{information}
{Paradyn cannot enable any more metric/focus pairs for this client.  This is due to a upper bound on the number of pairs the client can enable at one time.  Typically, visualization processes have an upper bound on enabled pairs.
}
}

set numPdErrors 97
