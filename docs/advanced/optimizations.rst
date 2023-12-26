
==============================
Optimizing Dyninst Performance
==============================

This section describes how to tune Dyninst for optimum performance.
During the course of a run, Dyninst will perform several types of
analysis on the binary, make safety assumptions about instrumentation
that is inserted, and rewrite the binary (perhaps several times). Given
some guidance from the user, Dyninst can make assumptions about what
work it needs to do and can deliver significant performance
improvements.

There are two areas of Dyninst performance users typically care about.
First, the time it takes Dyninst to parse and instrument a program. This
is typically the time it takes Dyninst to start and analyze a program,
and the time it takes to modify the program when putting in
instrumentation. Second, many users care about the time instrumentation
takes in the modified mutatee. This time is highly dependent on both the
amount and type of instrumentation put it, but it is still possible to
eliminate some of the Dyninst overhead around the instrumentation.

The following subsections describe techniques for improving the
performance of these two areas.

Optimizing Mutator Performance
------------------------------

CPU time in the Dyninst mutator is usually consumed by either parsing or
instrumenting binaries. When a new binary is loaded, Dyninst will
analyze the code looking for instrumentation points, global variables,
and attempting to identify functions in areas of code that may not have
symbols. Upon user request, Dyninst will also parse debug information
from the binary, which includes local variable, line, and type
information.

Since Dyninst 10.0.0, Dyninst supports parsing binaries in parallel,
which significantly improve the analysis speed. We typically have about
4X speedup when analyzing binaries with 8 threads. By default, Dyninst
will use all the available cores on your system. Please set environment
variable OMP_NUM_THREADS to the number of desired threads.

Debugging information is lazily parsed separately from the rest of the
binary parsing. Accessing line, type, or local variable information will
cause Dyninst to parse the debug information for all three of these.

Another common source of mutator time is spent re-writing the mutatee to
add instrumentation. When instrumentation is inserted into a function,
Dyninst may need to rewrite some or all of the function to fit the
instrumentation in. If multiple pieces of instrumentation are being
inserted into a function, Dyninst may need to rewrite that function
multiple times.

If the user knows that they will be inserting multiple pieces of
instrumentation into one function, they can batch the instrumentation
into one bundle, so that the function will only be re-written once,
using the BPatch_process::beginInsertionSet and
BPatch_­process::end­Inser­tion­Set functions (see section 4.4). Using
these functions can result in a significant performance win when
inserting instrumentation in many locations.

To use the insertion set functions, add a call to beginInsertionSet
before inserting instrumentation. Dyninst will start buffering up all
instrumentation insertions. After the last piece of instrumentation is
inserted, call finalizeInsertionSet, and all instrumentation will be
atomically inserted into the mutatee, with each function being rewritten
at most once.

Optimizing Mutatee Performance
------------------------------

As instrumentation is inserted into a mutatee, it will start to run
slower. The slowdown is heavily influenced by three factors: the number
of points being instrumented, the instrumentation itself, and the
Dyninst overhead around each piece of instrumentation. The Dyninst
overhead comes from pieces of protection code (described in more detail
below) that do things such as saving/restoring registers around
instrumentation, checking for instrumentation recursion, and performing
thread safety checks.

The factor by which Dyninst overhead influences mutatee run-time depends
on the type of instrumentation being inserted. When inserting
instrumentation that runs a memory cache simulator, the Dyninst overhead
may be negligible. On the other-hand, when inserting instrumentation
that increments a counter, the Dyninst overhead will dominate the time
spent in instrumentation. Remember, optimizing the instrumentation being
inserted may sometimes be more important than optimizing the Dyninst
overhead. Many users have had success writing tools that make use of
Dyninst’s ability to dynamically remove instrumentation as a performance
improvement.

The instrumentation overhead results from safety and correctness checks
inserted by Dyninst around instrumentation. Dyninst will automatically
attempt to remove as much of this overhead as possible, however it
sometimes must make a conservative decision to leave the overhead in.
Given additional, user-provided information Dyninst can make better
choices about what safety checks to leave in. An unoptimized
post-Dyninst 5.0 instrumentation snippet looks like the following:

+----------------------------------+----------------------------------+
| **Save General Purpose           | In order to ensure that          |
| Registers**                      | instrumentation doesn’t corrupt  |
|                                  | the program, Dyninst saves all   |
|                                  | live general purpose registers.  |
+----------------------------------+----------------------------------+
| **Save Floating Point            | Dyninst may decide to separately |
| Registers**                      | save any floating point          |
|                                  | registers that may be corrupted  |
|                                  | by instrumentation.              |
+----------------------------------+----------------------------------+
| **Generate A Stack Frame**       | Dyninst builds a stack frame for |
|                                  | instrumentation to run under.    |
|                                  | This provides the illusion to    |
|                                  | instrumentation that it is       |
|                                  | running as its own function.     |
+----------------------------------+----------------------------------+
| **Calculate Thread Index**       | Calculate an index value that    |
|                                  | identifies the current thread.   |
|                                  | This is primarily used as input  |
|                                  | to the Trampoline Guard.         |
+----------------------------------+----------------------------------+
| **Test and Set Trampoline        | Test to see if we are already    |
| Guard**                          | recursively executing under      |
|                                  | instrumentation, and skip the    |
|                                  | user instrumentation if we are.  |
+----------------------------------+----------------------------------+
| **Execute User Instrumentation** | Execute any BPatch_snippet code. |
+----------------------------------+----------------------------------+
| **Unset Trampoline Guard**       | Marks the this thread as no      |
|                                  | longer being in instrumentation  |
+----------------------------------+----------------------------------+
| **Clean Stack Frame**            | Clean the stack frame that was   |
|                                  | generated for instrumentation.   |
+----------------------------------+----------------------------------+
| **Restore Floating Point         | Restore the floating point       |
| Registers**                      | registers to their original      |
|                                  | state.                           |
+----------------------------------+----------------------------------+
| **Restore General Purpose        | Restore the general purpose      |
| Registers**                      | registers to their original      |
|                                  | state.                           |
+----------------------------------+----------------------------------+

Dyninst will attempt to eliminate as much of its overhead as is
possible. The Dyninst user can assist Dyninst by doing the following:

-  **Write BPatch_snippet code that avoids making function calls.**
   Dyninst will attempt to perform analysis on the user written
   instrumentation to determine which general purpose and floating point
   registers can be saved. It is difficult to analyze function calls
   that may be nested arbitrarily deep. Dyninst will not analyze any
   deeper than two levels of function calls before assuming that the
   instrumentation clobbers all registers and it needs to save
   everything.

..

   In addition, not making function calls from instrumentation allows
   Dyninst to eliminate its tramp guard and thread index calculation.
   Instrumentation that does not make a function call cannot recursively
   execute more instrumentation.

-  **Call BPatch::setTrampRecursive(true) if instrumentation cannot
   execute recursively.** If instrumentation must make a function call,
   but will not execute recursively, then enable trampoline recursion.
   This will cause Dyninst to stop generating a trampoline guard and
   thread index calculation on all future pieces of instrumentation. An
   example of instrumentation recursion would be instrumenting a call to
   write with instrumentation that calls printf—write will start calling
   printf printf will re-call write.

-  **Call BPatch::setSaveFPR(false) if instrumentation will not clobber
   floating point registers**. This will cause Dyninst to stop saving
   floating point registers, which can be a significant win on some
   platforms.

-  **Use simple BPatch_snippet objects when possible**. Dyninst will
   attempt to recognize, peep-hole optimize, and simplify frequently
   used code snippets when it finds them. For example, on x86 based
   platforms Dyninst will recognize snippets that do operations like
   ‘var = constant’ or ‘var++’ and turn these into optimized assembly
   instructions that take advantage of CISC machine instructions.

-  **Call BPatch::setInstrStackFrames(false) before inserting
   instrumentation that does not need to set up stack frames. Dyninst
   allows you to force stack frames to be generated for all
   instrumentation. This is useful for some applications (e.g.,
   debugging your instrumentation code) but allowing Dyninst to omit
   stack frames wherever possible will improve performance. This flag is
   false by default; it should be enabled for as little instrumentation
   as possible in order to maximize the benefit from optimizing away
   stack frames.**

-  **Avoid conditional instrumentation wherever possible.** Conditional
   logic in your instrumentation makes it more difficult to avoid saving
   the state of the flags.

-  **Avoid unnecessary instrumentation.** Dyninst provides you with all
   kinds of information that you can use to select only the points of
   actual interest for instrumentation. Use this information to
   instrument as selectively as possible. The best way to optimize your
   instrumentation, ultimately, is to know *a priori* that it was
   unnecessary and not insert it.