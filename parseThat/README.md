# parseThat

## Introduction

parseThat is an application that provides a rigorous and robust test of
DyninstAPI on arbitrary binaries.  Geared mainly for debugging purposes,
parseThat generates copious output for each dyninstAPI feature it uses.

These output logs can be used to quickly locate the source of parsing or
instrumentation bugs, in the rare instance when they occur.

## Configuration

Autoconf scripts are used to gather information about the target platform
before building.  The top level Makefile, however, will issue configure commands.
If a custom configuration is desired, first do a top-level build (to ensure that 
the build directory is created).  Then "cd $PLATFORM" and issue the command:

	../configure <custom config options here>

By default, the parseThat binary will be placed in the build directory.
Use the --prefix flag to install it somewhere else.  For example, to
install parseThat in /usr/local/bin:

	./configure --prefix=/usr/local/bin

There are a few variables which allow you to control how parseThat is
built.

	PLATFORM
	The configuration scripts will attempt to guess your target
	platform, but it's sometime helpful to specify your build
	platform manually.  See the documentation for DyninstAPI in
	dyninst/dyninstAPI/README for valid values of this variable.

	DYNINST_ROOT
	The various headers and libraries for DyninstAPI may not be
	installed in a standard location.  Use this environment
	variable to manually specify where you placed a source or
	binary distribution of DyninstAPI.

## Building and Installing

To build parseThat from scratch, issue the make command.  This will create
a platform-specific build directory, configure the Makefile, and build 
parseThat.  The top level Makefile (not platform specific) just relays make
commands to the platform specific Makefile in the $PLATFORM directory.

Once successfully built, invoking GNU make with the "install" target will
copy the binary to its final destination.

## Running

Documentation on running parseThat is encoded within the binary itself.
It can be viewed by simply running parseThat with the --help flag.
