
#
# $Log: errorList.tcl,v $
# Revision 1.1  1994/07/07 05:59:33  karavan
# initial version.
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
set numPdErrors 9
