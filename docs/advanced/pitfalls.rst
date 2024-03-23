
===============
Common pitfalls
===============

These are some common pitfalls that users
have reported when using the Dyninst system. Many of these are either
due to limitations in the current implementations, or reflect design
decisions that may not produce the expected behavior from the system.

Attach followed by detach
-------------------------

If a mutator attaches to a mutatee, and immediately exits, the current
behavior is that the mutatee is left suspended. To make sure the
application continues, call detach with the appropriate flags.

Attaching to a program that has already been modified by Dyninst
----------------------------------------------------------------

If a mutator attaches to a program that has already been modified by a
previous mutator, a warning message will be issued. We are working to
fix this problem, but the correct semantics are still being specified.
Currently, a message is printed to indicate that this has been
attempted, and the attach will fail.

Dyninst is event-driven
-----------------------

Dyninst must sometimes handle events that take place in the mutatee, for
instance when a new shared library is loaded, or when the mutatee
executes a fork or exec. Dyninst handles events when it checks the
status of the mutatee, so to allow this the mutator should periodically
call one of the functions BPatch::pollForStatusChange,
BPatch::wait­ForStatusChange, BPatch_thread::isStopped, or
BPatch_­thread::is­Termin­ated.
