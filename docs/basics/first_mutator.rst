
==================
Your First Mutator
==================

In this section, we describe the steps needed to compile your mutator
and mutatee programs and to run them. First we give you an overview of
the major steps and then we explain each one in detail.

Overview of Major Steps
-----------------------

To use Dyninst, you have to:

(1) *Build and install DyninstAP:* DyninstAPI can be installed a package
    system such as Spack or can be compiled from source. Our github
    webpage contains detailed instructions for installing Dyninst:
    https://github.com/dyninst/dyninst.

(2) *Create a mutator program (Section 6.2):* You need to create a
    program that will modify some other program. For an example, see the
    mutator shown in Appendix A.

(3) *Set up the mutatee (Section 6.3):* On some platforms, you need to
    link your application with Dyninst’s run time instrumentation
    library. [**NOTE**: This step is only needed in the current release
    of the API. Future releases will eliminate this restriction.]

(4) *Run the mutator (Section 6.4):* The mutator will either create a
    new process or attach to an existing one (depending on the whether
    createProcess or attachProcess is used).

Sections 6.2 through 6.4 explain these steps in more detail.

Creating a Mutator Program
--------------------------

The first step in using Dyninst is to create a mutator program. The
mutator program specifies the mutatee (either by naming an executable to
start or by supplying a process ID for an existing process). In
addition, your mutator will include the calls to the API library to
modify the mutatee. For the rest of this section, we assume that the
mutator is the sample program given in Appendix A - Complete Examples.

The following fragment of a Makefile shows how to link your mutator
program with the Dyninst library on most platforms:

.. code-block:: make

   
   # DYNINST_INCLUDE and DYNINST_LIB should be set to locations
   # where Dyninst header and library files were installed, respectively

   retee.o: retee.c
      $(CC) -c $(CFLAGS) -I$(DYNINST_INCLUDE) retee.c –std=c++11x
   
   retee: retee.o
      $(CC) retee.o -L$(DYNINST_LIB) -ldyninstAPI -o retee –std=c++11x

On Linux, the options -lelf and -ldw may be required at the link step.
You will also need to make sure that the LD_LIBRARY_PATH environment
variable includes the directory that contains the Dyninst shared
library.

Since Dyninst uses the C++11x standard, you will also need to enable
this option for your compiler. For GCC versions 4.3 and later, this is
done by specifying -std=c++0x. For GCC versions 4.7 and later, this is
done by specifying -std=c++11. Some of these libraries, such as libdwarf
and libelf, may not be standard on various platforms. Check the README
file in dyninst/dyninstAPI for more information on where to find these
libraries.

Under Windows NT, the mutator also needs to be linked with the dbghelp
library, which is included in the Microsoft Platform SDK. Below is a
fragment from a Makefile for Windows NT:

.. code-block:: make

   
   # DYNINST_INCLUDE and DYNINST_LIB should be set to locations
   # where Dyninst header and library files were installed, respectively

   CC = cl
   retee.obj: retee.c
      $(CC) -c $(CFLAGS) -I$(DYNINST_INCLUDE)/h

   retee.exe: retee.obj
      link -out:retee.exe retee.obj $(DYNINST_LIB)\libdyninstAPI.lib dbghelp.lib

Setting Up the Application Program (mutatee)
--------------------------------------------

On most platforms, any additional code that your mutator might need to
call in the mutatee (for example files containing instrumentation
functions that were too complex to write directly using the API) can be
put into a dynamically loaded shared library, which your mutator program
can load into the mutatee at runtime using the loadLibrary member
function of BPatch_process.

To locate the runtime library that Dyninst needs to load into your
program, an additional environment variable must be set. The variable
DYNINSTAPI_RT_LIB should be set to the full pathname of the run time
instrumentation library, which should be:

.. note::

   DYNINST_LIB should be set to the location where Dyninst library files were installed

   | ``$(DYNINST_LIB)/libdyninstAPI_RT.so`` (UNIX)
   | ``%DYNINST_LIB/libdyninstAPI_RT.dll`` (Windows)

Running the Mutator
-------------------

At this point, you should be ready to run your application program with
your mutator. For example, to start the sample program shown in Appendix
A - Complete Examples:

.. code-block:: bash

   $ retee foo <pid>