#
# Makefile for dyninst test programs on Windows NT.
#
# $Id: Makefile,v 1.1 2008/06/18 20:16:40 carl Exp $
#

# Define any symbols needed to invoke configuration changes in nmake.config

TO_CORE=..\..
NO_OPT_FLAG=true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

!include $(TO_CORE)/nmake.config
DEFCFLAGS = $(DEFCFLAGS) -MD # ccw 12 july 2001
LDFLAGS = $(LDFLAGS) /NODEFAULTLIB:LIBC.lib  # ccw 12 july 2001
# Now make any necessary architecture specific changes to variables:

#Support for GNU compilers is being dropped since it seems to be rarely used.
#M_GCC = gcc
#M_GXX = g++
NATIVE_CC = VC
NATIVE_CXX = VC++
TEST_LINKS = libdyninstAPI.lib libtestSuite.lib

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

!include ../nmake.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.
!include $(TO_CORE)/nmake.program.tmpl
!include ./nmake.mutators.gen
!include ./nmake.solo_mutatee.gen
