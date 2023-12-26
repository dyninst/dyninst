
========
Overview
========

The normal cycle of developing a program is to edit the source code,
compile it, and then execute the resulting binary. However, sometimes
this cycle can be too restrictive. We may wish to change the program
while it is executing or after it has been linked, thus avoiding the
process of re-compiling, re-linking, or even re-executing the program to
change the binary. At first, this may seem like a bizarre goal, however,
there are several practical reasons why we may wish to have such a
system. For example, if we are measuring the performance of a program
and discover a performance problem, it might be necessary to insert
additional instrumentation into the program to understand the problem.
Another application is performance steering; for large simulations,
computational scientists often find it advantageous to be able to make
modifications to the code and data while the simulation is executing.

This document describes an Application Program Interface (API) to permit
the insertion of code into a computer application that is either running
or on disk. The API for inserting code into a running application,
called dynamic instrumentation, shares much of the same structure as the
API for inserting code into an executable file or library, known as
static instrumentation. The API also permits changing or removing
subroutine calls from the application program. Binary code changes are
useful to support a variety of applications including debugging,
performance monitoring, and to support composing applications out of
existing packages. The goal of this API is to provide a machine
independent interface to permit the creation of tools and applications
that use runtime and static code patching. The API and a simple test
application are described in [1]. This API is based on the idea of
dynamic instrumentation described in [3].