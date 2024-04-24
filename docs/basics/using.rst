.. _`sec-basic-using-dyninst`:

Writing your first tool
#######################

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

Inserting code into a running application,
called dynamic instrumentation, shares much of the same structure as the
API for inserting code into an executable file or library, known as
static instrumentation. The Dyninst toolkits also permit changing or removing
subroutine calls from the application program. Binary code changes are
useful to support a variety of applications including debugging,
performance monitoring, and to support composing applications out of
existing packages. The goal of Dyninst is to provide a machine-independent
interface to permit the creation of tools and applications
that use runtime and static code patching.


Abstractions
************

Mutator
  An application written by the user that utilizes Dyninst's API to read,
  analyze, and/or manipulate a binary.

Mutatee
  The binary file analyzed by the mutator.


Creating a Mutator Program
**************************

An example of basic usage is printing symbols names from a mutatee. The code below shows one
possible way of doing this using :ref:`sec:parseapi-intro`.

.. rli:: https://raw.githubusercontent.com/dyninst/examples/master/symtabAPI/printSymbols.cpp
  :language: cpp
  :linenos:

This mutator can be compiled using the CMakeLists.txt

.. code:: CMake

  find_package(Dyninst 13.0.0 REQUIRED COMPONENTS parseAPI)

  add_executable(printSymbols printSymbols.cpp)

  target_link_libraries(printSymbols PRIVATE Dyninst::parseAPI)

.. code:: shell

  $ cmake /path/to/printSymbols -DDyninst_DIR=/path/to/Dyninst/lib/cmake/Dyninst
  $ cmake --build .

See :ref:`sec-importing` for details on building your application using Dyninst.

Running a Mutator Program
*************************

Before executing a mutator, the Dyninst libraries need to be in the ``LD_LIBRARY_PATH`` and
``DYNINSTAPI_RT_LIB`` needs to contain the path to ``libdyninstAPI_RT.so``.

Assuming a simple mutatee program in `test.cpp`

.. code:: cpp

  int bar=3;

built as a shared library (assuming a Linux environment)

.. code:: shell

  $ g++ -o libtest.so -fPIC -shared test.cpp

can be consumed by the mutator program from above using

.. code:: shell

  $ export LD_LIBRARY_PATH=/path/to/Dyninst/lib:$LD_LIBRARY_PATH
  $ export DYNINSTAPI_RT_LIB=/path/to/Dyninst/lib/libdyninstAPI_RT.so
  $ ./printSymbols libtest.so

More code examples can be found in :ref:`sec-examples`.
