
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
set numPdErrors 20
