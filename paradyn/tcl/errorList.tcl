# $Id: errorList.tcl,v 1.57 2003/04/27 04:17:34 schendel Exp $

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
# } {error type -> information, warning, serious, fatal
# } {
# Explanation -> Detailed explanation of the error and possible actions to be
#		 taken by the user
# }}
#
# Call example: 
# 	showErrorCallback(27, "Executable file /p/paradyn/weird not found");
#

# COMMENT: the "information" error type doesn't actually correspond to an
# error at all, but rather an informational message which is handled in a
# rather similar way (though differentiated in how they are presented).
# NOTE: many former "information" messages have been re-typed "warning"s.

# NOTE: the default explanations should generally _not_ have a newline in them.
# The tk text widget will word wrap just fine on its own.  Putting in explicit
# newlines forces a line break at certain points in the text widget---and who's
# to say that it will be in the right place since the user can resize the
# error dialog box to different widths?

# IMPORTANT:  if you add/delete an error, change the error count in
#             getNumPdErrors at the end of this file

set pdError(1) {
{Application Process found for machine without paradynd}
{paradynd}
{serious error}
{An application process was found to be running on a machine that had no\
paradynd process running.  This is a serious error that indicates either a\
paradynd process could not be started, or that it might have died.  This\
error should be considered an indication of a bug in the tool.}
} 

set pdError(2) {
{Data for unknown metric id}
{dm}
{serious error}
{Data has arrived from a paradynd process for an unknown metric id.  This\
is a serious error that indicates a bug in the paradyn/paradynd interface.}
} 

set pdError(3) {
{Unable to find metric component for sample.}
{dm}
{serious error}
{A sample value has arrive for a metric from a paradynd, but the paradyn\
process was not expecting a value from this process.  This is a serious\
internal consistency failure of the paradyn/paradynd interface.}
} 

set pdError(4) {
{Unable to connect to new paradyn daemon process.}
{paradynd}
{serious error}
{A request had arrived to start a new paradyn daemon process on a remote\
machine (either from the user or the system based on adding new hosts), and\
the paradyn user process was unable to rendezvous with the paradynd\
process.  This could indicate a network failure, the paradynd process not\
being installed on the remote machine, or a file permission problem.}
} 

set pdError(5) {
{paradynd process has died}
{paradynd}
{warning}
{A paradynd process has died somewhere in the system.  This indicates\
either a network failure, a host failure, or a bug in the paradynd process.}
} 

set pdError(6) {
{Unable to start paradynd}
{dm}
{warning}
{A request to start a new application process on a machine required that a\
new paradyn daemon process be started on that machine.  The attempt to\
start that new paradynd process failed.  This is a continuable error, but\
does mean that the request application process will NOT be started.  This\
error can happen if the path for the paradynd is incorrect, if the paradynd\
binary is not installed on that machine, or if the machine or network is down.}
} 

set pdError(7) {
{Auto refinement already enabled}
{pc}
{serious error}
{An attempt to enable automatic refinement was made while automated\
refinement was already being attempted.}
} 

set pdError(8) {
{Unable to find search history graph node}
{pc}
{warning}
{An attempt to lookup a search history graph node failed.  The passed\
integer name of the node was not found in the list of nodes.}
} 

set pdError(9) {
{Search history graph ancestor not true}
{pc}
{warning}
{An attempt to set the current refinement to a node failed because one of\
the ancestors of that node is false.  To manually select a SHG node, you\
must select a node which is true.  In addition, all of its ancestors back\
to the root must also be true.}
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
{warning}
{An application process has exited. This situation may be produced, for\
example, by an unsuccessful request of memory made by this process, or it\
could be possible that the application just finished.}
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
{A request to start a new visualization process has failed. Some possible\
explanations are:
(1) the executable for this visi is not well installed, and you should\
check whether the executable is in the right place,
(2) the process you just started is not a visi process, or
(3) Paradyn and the visi process are communicating in an incompatible\
fashion; e.g. mixing different release numbers of Paradyn and the visi.}
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
{Possible causes: bufferSize out of range in VISIthreadDataCallback and\
remove() in VISIthreadmain. 
Please report this error to paradyn@cs.wisc.edu}
} 

set pdError(17) {
{Adding new metrics and/or foci failed}
{vi}
{warning}
{An incomplete or invalid metric or focus list was returned as a result of\
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
{An unrecoverable error occurred within a visi manager function.
Please report this error to paradyn@cs.wisc.edu}
} 

set pdError(21) {
{Tcl Command Failure}
{ui}
{fatal error}
{The tcl interpreter has failed. Bad pointer "newptr". 
Please report this error to paradyn@cs.wisc.edu}
} 

set pdError(22) {
{Tcl Command Failure}
{ui}
{fatal error}
{The tcl interpreter has failed unexpectedly (getMetsAndRes in\
UIM::chooseMetricsandResources).
Please report this error to paradyn@cs.wisc.edu}
} 

set pdError(23) {
{Read error}
{paradynd }
{warning}
{Read error in application process.}
}

set pdError(24) {
{Unable to read tcl start-up script}
{ui}
{warning}
{A tcl error occurred finding or reading the tcl script specified on the\
paradyn command line with the -s option.}
} 

set pdError(25) {
{Unable to define specified process}
{ui}
{warning}
{An error occurred while attempting to define an application.\
Check directory, check command, and try again.}
} 

set pdError(26) {
{unable to attach to specified process}
{ui}
{warning}
{An error occurred while attempting to attach to an existing application.\
Check the pid, check the machine name (if any), and try again.}
} 

set pdError(27) {
{Executable not found.}
{paradynd}
{warning}
{The executable you are trying to run does not exist.\
Check your executable filename and path again!}
}

set pdError(28) {
{Unable to find symbol.}
{paradynd}
{warning}
{Sorry, no more information available.}
}

set pdError(29) {
{Function has bad address.}
{paradynd}
{warning}
{Sorry, no more information available.}
}

set pdError(30) {
{Incorrect version number.}
{paradynd}
{warning}
{Sorry, no more information available.}
}

set pdError(31) {
{Internal symbol DYNINSTfirst not found.}
{paradynd}
{serious error}
{You have not properly linked your application with the paradyn dyninst\
library.  Please refer to the manual pages in order to check how to do this.}
}

set pdError(32) {
{Internal symbol DYNINSTend not found.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(33) {
{Could not find version number in instrumentation.}
{paradynd}
{warning}
{Your program might have been linked with the wrong version of the paradyn\
dyninst library, or it could be a non executable binary file.}
}

set pdError(34) {
{Error function without module.}
{paradynd}
{warning}
{Sorry, no more information available.}
}

set pdError(35) {
{Unable to open PIF file.}
{paradynd}
{warning}
{Sorry, no more information available.}
}

#set pdError(36) {
#{Internal error: non-aligned length received on traceStream.}
#{paradynd}
#{serious error}
#{Please report this error to paradyn@cs.wisc.edu}
#}

set pdError(36) {
{Received invalid data on trace stream.}
{paradynd}
{serious error}
{Paradyn received invalid data from a process it was tracing.\
There may be many different causes to this problem:
1. a bug in the application being traced;
2. a bug in Paradyn DYNINST library;
3. a bug in the instrumentation code inserted into the application;
4. a bug in the paradyn daemon.
Please report this error to paradyn@cs.wisc.edu}
}

set pdError(37) {
{Internal error: wrong record type on sid}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(38) {
{Error in forwarding signal}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(39) {
{Internal error: unknown process state }
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(40) {
{Internal error: unable to detach PID}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(41) {
{Unable to open file.}
{paradynd}
{warning}
{Sorry, no more information available.}
}

set pdError(42) {
{Internal error: unable to parse executable.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(43) {
{Internal error: unable to get loader info about process. }
{paradynd}
{serious error}
{Internal error -- the ptrace PT_LDINFO call failed. 
Please report this error to paradyn@cs.wisc.edu}
}

set pdError(44) {
{Internal error: error reading header}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(45) {
{Internal error: problem with executable header file.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(46) {
{Program not statically linked.}
{paradynd}
{warning}
{Sorry, no more information available.}
}

set pdError(47) {
{Dump core failed.}
{paradynd}
{warning}
{A paradyn daemon could not dump the core image of a process. This problem\
can happen because paradyn could not open the file, or it could not write\
to the file.}
}

set pdError(48) {
{Symbol table out of order, use -Xlinker -bnoobjreorder}
{paradynd}
{warning}
{Sorry, no more information available.}
}

set pdError(49) {
{Error reading executable file.}
{paradynd}
{warning}
{Sorry, no more information available.}
}

set pdError(50) {
{Internal error: Cannot find file in inst-power.C}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(51) {
{Internal error: In forkNodeProcesses, parent id unknown.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(52) {
{Internal error: Branch too far.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(53) {
{Internal error: Program text + data is too big for dyninst.}
{paradynd}
{fatal error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(54) {
{Program text + data could be too big for dyninst.}
{paradynd}
{warning}
{Sorry, no more information available.}
}

set pdError(55) {
{Internal error: Unsupported return.}
{paradynd}
{serious error}
{Internal error. Please, report this error to paradyn@cs.wisc.edu}
}

set pdError(56) {
{Internal error: exec failed in paradynd to start paradyndCM5.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(57) {
{Internal error: could not write all bytes.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(58) {
{Internal error: unable to find process.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(59) {
{Internal error: there are no processes known to this daemon.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(60) {
{Internal error: unable to find thread.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(61) {
{Internal error: disableDataCollection mid not found.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(62) {
{Internal error: cannot continue PID.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(63) {
{Internal error: cannot pause PID.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(64) {
{No function calls in procedure.}
{paradynd}
{warning}
{No function calls where found in current procedure.}
}

set pdError(65) {
{Sample not for valid metric instance.}
{paradynd}
{warning}
{Sorry, no more information available.}
}

set pdError(66) {
{Internal error: inferior heap overflow.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(67) {
{Internal error: attempt to free already freed heap entry.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(68) {
{Internal error: unable to start file.}
{paradynd}
{warning}
{Sorry, no more information available.}
}

set pdError(69) {
{Internal error: ptrace error.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(70) {
{Internal error: execv failed. }
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(71) {
{Internal error: vfork failed.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(72) {
{Internal error: unable to stat.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(73) {
{Internal error: could not (un)marshall parameters, dumping core.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(74) {
{Internal error: protocol verification failed.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(75) {
{Internal error: cannot do sync call.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(76) {
{Internal error: unknown message tag. }
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(77) {
{Internal error: handle error called for wrong err_state.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(78) {
{Internal error: problem stopping process.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(79) {
{Internal error: unable to find addr of DYNINSTobsCostLow.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(80) {
{Internal error: unable to find addr of callee process.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(81) {
{Cannot start process on specified host.}
{dm}
{warning}
{This error may be produced by a wrong host name. Paradyn cannot create the\
process on the host you are specifying.}
}

set pdError(82) {
{Trying to run a thread that is not ready yet.}
{dm}
{warning}
{You are trying to run a process that it is still being created.\
Please wait and try again.}
}

set pdError(83) {
{Internal error: Tcl interpreter failed in routine changeApplicState.}
{ui}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(84) {
{Cannot create new paradyn daemon process.}
{dm}
{serious error}

{An error was detected when a paradyn daemon was being created.\
Possible explanations for this problem are:
(1) unknown host,
(2) it is not possible to establish connection with the specified host, or
(3) incompatible versions of Paradyn and Paradynd are being used\
(e.g. release 1.0 of one and release 1.1 of the other).}
}

set pdError(85) {
{Error found in metrics specified in the Paradyn configuration file.}
{pdMain}
{warning}
{An error was detected when Paradyn was reading the metrics described in\
the Paradyn configuration file.}
}

set pdError(86) {
{Cannot enable metric}
{dm}
{warning}
{Paradyn cannot enable this particular metric. This might be due to\
constraints in the definition of the metric (e.g. the metric is restricted\
to the whole program and we have selected a particular process), or because\
a resource component of the focus was excluded in a Paradyn configuration\
file by the "exclude" option.  
Another cause could be if the enable request is for the current phase, and\
if a new phase was started in the middle of the enable request, then some\
metric/focus pairs may not have been enabled for the old phase.}
}

set pdError(87) {
{Error in external visualization process. }
{vi}
{warning}
{An error has occurred during the execution of an external visualization\
process.}
}

set pdError(88) {
{Must define an application before starting the Performance Consultant.}
{ui}
{warning}
{No additional information available.}
}

set pdError(89) {
{Command line is missing.}
{vi}
{warning}
{The command line is missing from your customized PCL file. You cannot\
start a process without specifing a command line.}
}

set pdError(90) {
{The Paradyn daemon you have specified does not exist.}
{vi}
{warning}
{The Paradyn daemon you have specified does not exist. Paradyn cannot start\
without a valid Paradyn daemon. Please try again.}
}

set pdError(91) {
{Paradyn daemon is missing in PCL file.}
{vi}
{warning}
{The Paradyn daemon is missing from your customized PCL file. You need to\
specify a valid Paradyn daemon in order to start an application on a\
particular machine.}
}

## MDL errors
set pdError(92) {
{Error while evaluating metric.}
{paradynd}
{warning}
{A error was found while evaluating a metric for a focus.  The metric\
cannot be enabled for this focus.
}
} 

set pdError(93) {
{Metric has no constraint that matches focus.}
{paradynd}
{warning}
{You tried to enable a metric for a focus, but the metric has no constraint\
for this focus. Check your focus selection from the where axis.}
}

set pdError(94) {
{Too many arguments to function call in instrumentation code.}
{paradynd}
{serious error}
{An instrumentation request includes a function call with too many arguments.\
The maximum number of arguments that can be passed to a function is platform\
dependent.}
}

set pdError(95) {
{Unable to find symbol for metric definition.}
{paradynd}
{warning}
{Paradyn could not enable a metric because the metric definition has a\
function call or tries to read the value of a symbol with readSymbol, but\
paradyn could not find the symbol in the application symbol tables.}
}

set pdError(96) {
{Internal error: attempt to free non-defined heap entry.}
{paradynd}
{serious error}
{Internal error. Please report this error to paradyn@cs.wisc.edu}
}

set pdError(97) {
{Maximum number of metric/focus pairs have been enabled by this client}
{dm}
{warning}
{Paradyn cannot enable any more metric/focus pairs for this client.\
This is due to a upper bound on the number of pairs the client can\
enable at one time.  Typically, visualization processes have an upper\
bound on enabled pairs.}
}

set pdError(98) {
{paradynd has been terminated}
{pd}
{serious}
{The paradyn daemon has received a SIGTERM signal probably because the\
daemon has ran out of virtual memory. Please check the memory\
usage of your application.}
}

set pdError(99) {
{Paradyn daemon start-up info}
{dm}
{information}
{In most cases paradyn daemons are started automatically.\
Manual start-up is needed only when an rshd or rexecd\
is not available on the remote machine.}
}

set pdError(100) {
{Attempt to look up nonexistent symbol}
{paradynd}
{serious}
{An attempt was made to look up a function or variable,\
but no symbol with the requested name was found in the application.}
}

set pdError(101) {
{Internal error: unable to load paradyn run-time library.}
{paradynd}
{serious error}
{One possible explanation is that the call to dlopen made in order to\
load the paradyn run-time library has failed. Please check the content\
of the environment variable PARADYN_LIB, which should have the whole\
path name of the library, and make sure that the library is located in\
a directory that is readable by any user. This is a known problem if\
you are running an application on more than one host and you have the\
library located in a directory that has reading restrictions to all users.}
}

set pdError(102) {
{Internal error: shared memory segment request exceeded current limit.}
{paradynd}
{fatal error}
{A request for a shared memory segment failed for this process, since it\
would exceed the system-imposed limit in the OS kernel. You could try to\
fix this problem by terminating other applications or by not running\
so many processes on the same host.}
}

set pdError(103) {
{Unable to open file}
{paradyn}
{warning}
{Check whether the filename provided includes a valid path\
and that a file with that name can be (over-)written.}
}

set pdError(104) {
{Paradyn General Information}
{paradyn}
{information}
{Paradyn is a tool for measuring and analyzing the performance of\
parallel and distributed programs. Paradyn can measure large, long\
running programs and provides facilities for helping to automatically\
find performance problems in parallel programs. Paradyn operates on\
executable (a.out or .exe) files by dynamically inserting measurement\
code while the program is running. Paradyn can measure programs running\
on Solaris (SPARC), Linux (x86), AIX (Power), and Windows (x86),\
or heterogeneous combinations of these systems.\
Paradyn can also handle MPI applications on these platforms.

For further information go to our Web pages at:
        http://www.paradyn.org/

If you have any problems with installing or running Paradyn, or wish to\
report a bug, please contact us at paradyn@cs.wisc.edu. The information\
you provide will help us to improve future releases, and we try to\
respond promptly.}
}

set pdError(105) {
{Paradyn License Information}
{paradyn}
{information}
{Copyright (c) 1996-2003 Barton P. Miller (University of Wisconsin-Madison)

We provide the Paradyn Parallel Performance Tools (below described as\
"Paradyn") on an AS IS basis, and do not warrant its validity or\
performance. We reserve the right to update, modify, or discontinue\
this software at any time. We shall have no obligation to supply such\
updates or modifications or any other form of support to you.

This license is for research uses. For such uses, there is no charge.\
We define "research use" to mean you may freely use it inside your\
organization for whatever purposes you see fit. But you may not\
re-distribute Paradyn or parts of Paradyn, in any form source or binary\
(including derivatives), electronic or otherwise, to any other\
organization or entity without our permission.

(For other uses, please contact us at paradyn@cs.wisc.edu)

All warranties, including without limitation, any warranty of\
merchantability or fitness for a particular purpose, are hereby excluded.

By your use of Paradyn, you understand and agree that we (or any other\
person or entity with proprietary rights in Paradyn) are under no\
obligation to provide either maintenance services, update services,\
notices of latent defects, or correction of defects for Paradyn.

Even if advised of the possibility of such damages, under no circumstances\
shall we (or any other person or entity with proprietary rights in the\
software licensed hereunder) be liable to you or any third party for\
direct, indirect, or consequential damages of any character regardless of\
type of action, including, without limitation, loss of profits, loss of\
use, loss of good will, or computer failure or malfunction. You agree to\
indemnify us (and any other person or entity with proprietary rights in\
the software licensed hereunder) for any and all liability it may incur to\
third parties resulting from your use of Paradyn.}
}

set pdError(106) {
{Paradyn Release Information}
{paradyn}
{information}
{This Paradyn release has thorough multi-threaded support for AIX and\
Solaris, a front-end that now uses a premptively multithreaded thread \
package, and resurrected Paradyn support for handling of application fork and \
exec calls and many other improvements. 

Full release details are included in the Paradyn User's Guide.

Paradyn Parallel Performance Tools binary (executable) and source releases\
are available via ftp from:
        ftp://ftp.cs.wisc.edu/paradyn/releases/

If you want to be notified of future releases of Paradyn, please\
E-mail us at paradyn@cs.wisc.edu.}
}

set pdError(107) {
{Paradyn Version Information}
{paradyn}
{information}
{}
}

set pdError(108) {
{Internal error: procedure main cannot be instrumented.}
{paradynd}
{fatal error}
{Paradyn could not find or instrument procedure main. One possible explanation\
is that main has a jump to the middle of the instrumentation that Paradyn\
needs to insert. To avoid this, you can try to move any loops at the beginning\
of main to some other place. A future release will fix this problem.}
}

set pdError(109) {
{Unimplemented feature}
{paradynd}
{warning}
{You have attempted to use a feature that has not been implemented for\
this platform.}
}

set pdError(110) {
{Must define an application before creating a Call Graph.}
{ui}
{warning}
{No additional information available.}
}

set pdError(111) {
{Search type may not be changed after starting Performance Consultant.} 
{pc}
{warning}
{No additional information available.}
}

set pdError(112) {
{Run time library time retrieval function timer rollback.}
{rtlibrary}
{warning}
{Could be caused by a buggy time retrieval function\
(eg. gettimeofday or gethrtime) such that it returned a time that\
went backwards.  It's also possible that this is an error caused by\
a race condition.}
}

set pdError(113) {
{Unable to launch MPI job.}
{dm}
{serious error}
{Paradyn has failed to start MPI on the host you have specified. \
Please confirm that you have provided Paradyn with a valid MPI command \
and, if you intend to launch MPI on a remote machine, Paradyn can \
start a remoteShell on the remote machine. }
}

set pdError(114) {
{Using mpirun option with undefined behavior.}
{dm}
{warning}
{The behavior of Paradyn is undefined when mpirun is started with\
this option.  If you are using the -f[ile] option, you may have\
obscured executables which Paradyn will not be able to identify.}
}

set pdError(115) {
{Unable to start a visi until application is defined.}
{ui}
{warning}
{You must define an application before starting a data visualization. \
Paradyn needs to have an application defined to define its metrics and \
resources, which are needed in order to define a metric/focus pair for \
a visualization.}
}

set pdError(116) {
{Invalid group ID encountered.}
{rtlibrary}
{warning}
{An invalid group ID has been encountered, most likely from attempting \
to access function arguments from an unsupported point within the \
function.}
}

set pdError(117) {
{Instrumentation point conflict.}
{paradynd}
{serious error}
{A requested instrumentation point conflicts with a point that has \
already beeen created.  The conflict is caused by the fact that the \
instructions that would be replaced in order to instrument the point \
overlap those that are or would be replaced in order to instrument the \
previously created point.  In some cases you may be able to create the \
point if you do not also create the point it is in conflict with, but \
you will not be able to create both points.}
}

set pdError(118) {
{Point uninstrumentable.}
{paradynd}
{serious error}
{A requested instrumentation point cannot be created.  This error \
occurs when the requested point is located in certain parts of the \
code that cannot be instrumented, such as the delay slot of a branch \
on some architectures.}
}

set pdError(119) {
{Table visi undefined.}
{ui}
{warning}
{There should be at least one table type visi defined in \
configuration files in order to show performance data table \
of matched metric/focus.}
}

set pdError(120) {
{illegal metric/focus pair.}
{vm}
{warning}
{metric/focus pair in visi definition should keep to format as below\
name of legal metric, name of legal focus\
example: metfocus "cpu,/Code/anneal.c,/Machine,/SyncObject";\
.}
}

set pdError(121) {
{Unable to start terminal window}
{ui}
{fatal error}
{Paradyn was not able to start its terminal window.  Paradyn must start the \
terminal window to collect application output.  Please check that the \
termWin program is in your PATH and that you have permissions to run \
the program.}
}

set pdError(122) {
{dumpPatchedImage failed}
{paradynd}
{serious error}
{BPatch_thread::startSaveWorld() was not called and no save world \
data was collected or the directory could not be created.}
}

set pdError(123) {
{dumpPatchedImage: mutatee used dlopen, the mutated binary may fail.}
{paradynd}
{warning}
{The mutatee used dlopen, the mutated binary may not load the shared \
library into the correct place and may fail.}
}



set pdError(124) {
{loadLibrary(): dlopen failed}
{rtlibrary}
{serious error}
{The RPC call to DYNINSTloadLibrary in the runtime library failed because \
dlopen failed.}
}

set pdError(125) {
{Error while evaluating metric.}
{paradynd}
{warning}
{A metric has attempted to use an invalid hardware performance counter. }
} 

set pdError(126) {
{instrumentation insertion failed}
{paradynd}
{serious error}
{An instrumentation insertion request failed because instrumentation failed
to load into the inferior application.}
}

set pdError(127) {
{Unimplemented feature}
{paradynd}
{fatal error}
{You have attempted to use a feature that has not been implemented for\
this platform.}
}


#
# be sure to change this value if you add/delete an entry to the database
#
proc getNumPdErrors {} {
    return 125
}
