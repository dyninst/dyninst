# Code Coverage

### A simple code coverage tool built with DyninstAPI

The tool uses Dyninst to instrument every function in a program binary as well as
optionally instrumenting every basic block to record code coverage data.  

The goal is to demonstrate some of the capabilities of DyninstAPI.
It does very little processing of the raw code coverage data, just some basic
sorting when outputting the data. This tool should serve as the basis for
creating a more feature-rich code coverage tool using Dyninst.

The presence of debugging information in the program binary is recommended to
acquire the most useful information about code coverage, but it is not
required. Without debugging information, source file information will not be
available in the code coverage output.

This tool makes use of an instrumentation library, libInst, to collect the code
coverage data. The tool adds this library as a dependency to the input
executable and also inserts calls into this library at function entries and
basic block entries.

===============================================================================

How to use this example:

1. Make sure that the directory where the `code_coverage` binary is located is
   part of your loader's path

   For bash: `% export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH`
   
   For csch: `% setenv LD_LIBRARY_PATH .:${LD_LIBRARY_PATH}`

2. Execute `code_coverage` without any arguments to see the usage.

	$ ./code_coverage
	Input binary not specified.
	Usage: ./code_coverage [-bpsa] <binary> <output binary>
	    -b: Basic block level code coverage
	    -p: Print all functions (including functions that are never executed)
	    -s: Instrument shared libraries also
	    -a: Sort results alphabetically by function name

3. Pass the testcc executable as input, instrumenting basic blocks as
   well as the example shared library used by testcc, libexternal.so.

	$ ./code_coverage -sb ./testcc testcc.inst
	  [ Lots of output stating what the tool is doing ]

   You may notice that the tool skips some shared libraries. The default behavior
   of the tool is to not instrument standard libraries such as libc.

   The last command will output a rewritten version of testcc, testcc.inst. It will
   also overwrite the existing libexternal.so file with a rewritten version. The
   default Dyninst behavior when rewritting the shared libraries of an executable
   is to output the shared libraries in the same directory as the rewritten 
   executable.

4. Run the testcc.inst program to generate code coverage data. An abridged
   version of the output follows.

	% ./testcc.inst
	
	 ************************** Code Coverage ************************* 
	
	    1 : _init, testcc
	    1 : __do_global_dtors_aux, testcc
	    1 : frame_dummy, testcc
	    1 : __do_global_ctors_aux, testcc
	    1 : _fini, testcc
	    1 : two, testcc.c
	    1 : one, testcc.c
	    1 : main, testcc.c
	
	 ************** Code Coverage 8 out of 17 functions ************** 
	
	
	
	 ************************** Basic Block Coverage ************************* 
	
	[...snip...]
	 (two, testcc.c)
	 	    1 : 0x4006ab  
	 (one, testcc.c)
	 	    1 : 0x4006c0  
	 	    1 : 0x4006d6  
	 (__do_global_dtors_aux, testcc)
	 	    1 : 0x40063c  
	 (one, testcc.c)
	 	    1 : 0x4006e4  
	 (main, testcc.c)
	 	    1 : 0x4006f0  
	 (__do_global_dtors_aux, testcc)
	 	    1 : 0x400643  
	 (main, testcc.c)
	 	    1 : 0x40071a
	[...snip...]
	
	 ************** Basic Block Coverage 23 out of 69 blocks ************** 
	
	END OUTPUT
	
	The first set of data contains counts for the number of times each function was
	called. The second set of data contains counts for the number of times each
	basic block was entered.
	
	If you run the program again with a few command line arguments, you will see
	that the 'three' function in testcc.c was now called as well as functions in
	libtestcc.so.
	
	% ./testcc.inst 1 2
	
	 ************************** Code Coverage ************************* 
	
	    1 : _init, testcc
	    1 : three, testcc.c
	    1 : two, testcc.c
	    1 : one, testcc.c
	    1 : main, testcc.c
	    1 : otherFunction, libtestcc.so
	    1 : libFooFunction, libtestcc.so
	    1 : __do_global_dtors_aux, testcc
	    1 : frame_dummy, testcc
	    1 : __do_global_ctors_aux, testcc
	    1 : _fini, testcc
	
	 ************** Code Coverage 11 out of 17 functions ************** 
	
	[...snip...] basic block output not included
	END OUTPUT

   This output shows that more functions have been called with this new input set.
