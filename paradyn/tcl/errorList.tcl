
#
# $Log: errorList.tcl,v $
# Revision 1.3  1994/11/03 17:47:19  karavan
# more errors (less frequent :)
#
# Revision 1.32  1994/11/03  04:38:47  karavan
# created error #26
#
# Revision 1.31  1994/11/02  23:47:17  karavan
# created error #25
#
# Revision 1.30  1994/11/02  23:41:51  karavan
# created error #24
#
# Revision 1.29  1994/09/13  20:50:58  karavan
# created error #23
#
# Revision 1.28  1994/09/13  20:48:39  karavan
# created error #22
#
# Revision 1.27  1994/09/13  20:44:06  karavan
# created error #21
#
# Revision 1.26  1994/07/12  16:58:43  newhall
# created error #20
#
# Revision 1.25  1994/07/12  16:52:14  newhall
# created error #19
#
# Revision 1.24  1994/07/12  16:46:42  newhall
# created error #18
#
# Revision 1.23  1994/07/11  22:22:11  newhall
# created error #17
#
# Revision 1.22  1994/07/11  22:09:02  newhall
# created error #16
#
# Revision 1.21  1994/07/11  21:35:42  newhall
# created error #15
#
# Revision 1.20  1994/07/11  21:31:31  newhall
# *** empty log message ***
#
# Revision 1.19  1994/07/11  21:14:26  newhall
# created error #14
#
# Revision 1.18  1994/07/11  20:48:38  newhall
# created error #13
#
# Revision 1.17  1994/07/11  20:47:15  newhall
# created error #12
#
# Revision 1.16  1994/07/08  21:27:11  jcargill
# Deleted 3 bogus test errors
#
# Revision 1.15  1994/07/08  21:23:43  jcargill
# created error #13
#
# Revision 1.14  1994/07/08  21:19:18  jcargill
# created error #12
#
# Revision 1.13  1994/07/08  21:14:07  jcargill
# created error #11
#
# Revision 1.12  1994/07/08  03:02:56  karavan
# created error #10
#
# Revision 1.11  1994/07/08  02:52:55  karavan
# created error #
#
# Revision 1.10  1994/04/22  21:51:17  hollings
# created error #9
#
# Revision 1.9  1994/04/22  21:48:54  hollings
# created error #8
#
# Revision 1.8  1994/04/19  18:59:35  hollings
# created error #7
#
# Revision 1.7  1994/04/19  18:54:43  hollings
# created error #6
#
# Revision 1.6  1994/04/19  18:50:06  hollings
# created error #5
#
# Revision 1.5  1994/04/19  18:46:30  hollings
# created error #4
#
# Revision 1.4  1994/04/19  18:41:55  hollings
# created error #3
#
# Revision 1.3  1994/04/19  18:37:43  hollings
# created error #2
#
# Revision 1.2  1994/04/19  18:29:48  hollings
# created error #1
#
# Revision 1.1  1994/04/19  18:26:56  hollings
# Libray of error numbers in paradyn.
#
#


set pdError(1) {

{Application Process found for machine without paradynd
} {paradynd
} {serious
} {
An application processes was found to be running on a machine that had no
paradynd process running.  This is a serious error that indicates either a
paradynd process could not be started, or that it might have died.  This 
error should be considered an indication of a bug in the tool.
}} 

set pdError(2) {
{Data for unknown metric id
} {dm
} {serious
} {
Data has arrived from a paradynd process for an unknown metric id.  This is
a serious error that indicates a bug in the paradyn/paradynd interface.
}} 

set pdError(3) {
{Unable to find metric component for sample.
} {dm
} {serious
} {
A sample value has arrive for a metric from a paradynd, but the paradyn 
process was not expecting a value from this process.  This is a serious internal
consistancy failure of the paradyn/paradynd interface.
}} 

set pdError(4) {
{unable to connect to new paradyn daemon process.
} {paradynd
} {serious
} {
A request had arrived to start a new paradyn daemon process on a remote 
machine (either from the user or the system based on adding new hosts), and
the paradyn user process was unable to rendezvous with the paradynd process.
This could indicate a network failure, the paradynd process not being 
installed on the remote machine, or a file permission problem.
}} 

set pdError(5) {
{paradynd process has died
} {paradynd
} {information
} {
A paradynd process has died somewhere in the system.  This indicates either
a network failure, a host failure, or a bug in the paradyd process.
}} 

set pdError(6) {
{unable to start paradynd
} {dm
} {information
} {
A request to start a new application process on a machine required that a new
paradyn daemon process be started on that machine.  The attempt to start that
new paradyd process failed.  This is a continuable error, but does mean that
the request application process will NOT be started.  This error can happen
if the path for the paradynd is incorrect, if the paradynd binary is not
installed on that machine, or if the machine or network is down.
}} 

set pdError(7) {
{auto refinement already enabled
} {pc
} {serious
} {
An attempt to enable automatic refinement was made will automated refinement
was already being attempted.
}} 

set pdError(8) {
{unable to find search history graph node
} {pc
} {information
} {
An attempt to lookup a search history graph node failed.  The passed interger
name of the node was not found in the list of nodes.

}} 

set pdError(9) {
{search history graph ancestor not true
} {pc
} {information
} {
An attempt to set the current refinement to a node failed becuase one of the
ancesstors of that node is false.  To manually select a SHG node, you must
select a node which is true.  In addition, all of it's ancesstors back to
the root must also be true.
}} 

set pdError() {
{malloc failed
} {dm
} {fatal
} {
Attempt to call malloc returned an error within a data manager function.

}} 

set pdError(10) {
{malloc failure
} {dm
} {fatal
} {
Call to malloc failed within a data manager function.
}} 

set pdError(12) {
{malloc failed
} {vi
} {serious
} {
Call to malloc failed within a visi-thread function
}} 

set pdError(13) {
{thr_getspecific failed
} {vi
} {serious
} {
Call to thr_getspecific in a visi-thread function failed.
}} 

set pdError(14) {
{unable to start visualization process
} {vi
} {serious
} {
A request to start a new visualization process failed.  This is due
to a failure in RPCprocessCreate, msg_bind_buffered, or thr_setspecific.
}} 

set pdError(15) {
{unable to create performance stream
} {vi
} {serious
} {
An attempt to create a performance stream for a new visualizaiton
failed.
}} 

set pdError(16) {
{internal error
} {vi
} {serious
} {

}} 

set pdError(17) {
{Adding new metrics and/or foci failed 
} {vi
} {information
} {
An incomplete or invalid metric or focus list was returned as a
result of attempting to add metrics and/or foci to a visualization.
}} 

set pdError(18) {
{malloc failure
} {vm
} {fatal
} {
Call to malloc failed within a visi manager function.
}} 

set pdError(19) {
{strdup failure
} {vm
} {fatal
} {
Call to strdup failed within a visi manager function.
}} 

set pdError(20) {
{internal error
} {vm
} {fatal
} {
An unrecoverable error occurred within a visi manager function.
}} 

set pdError(21) {
{Tcl Command Failure
} {ui
} {fatal
} {
The tcl interpreter has failed unexpectedly.  This is a fatal error; 
this error needs to be diagnosed.
}} 

set pdError(22) {
{Tcl Command Failure
} {ui
} {fatal
} {
The tcl interpreter has failed unexpectedly.  This is a fatal error; 
this error needs to be diagnosed.
}} 

set pdError(23) {
{Tcl Command Failure
} {ui
} {fatal
} {
The tcl interpreter has failed unexpectedly.  This is a fatal error; 
this error needs to be diagnosed.
}} 

set pdError(24) {
{Unable to read tcl startup script
} {ui
} {information
} {
A tcl error occurred finding or reading the tcl script specified on the 
paradyn command line with the -s option.
}} 

set pdError(25) {
{Unable to define specified process
} {ui
} {information
} {
An error occurred while attempting to define an application.
}} 

set pdError(26) {
{tcl initialization for new resource display object failed.
} {ui
} {fatal
} {
Call to tcl command initRDO failed.
}} 
set numPdErrors 26
