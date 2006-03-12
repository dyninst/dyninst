#
# Makefile for dyninst test programs on Windows NT.
#
# $Id: Makefile,v 1.4 2006/03/12 23:33:02 legendre Exp $
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

CFLAGS = /Zi /D_DEBUG
#Support for GNU compilers is being dropped since it seems to be rarely used.
#M_GCC = gcc
#M_GXX = g++
NATIVE_CC = VC
NATIVE_CXX = VC++
TEST_LINKS = libdyninstAPI.lib libtestSuite.lib test_lib_templates-nt.obj

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

!include Makefile.vars

!include ../nmake.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.
!include $(TO_CORE)/nmake.program.tmpl

!include Makefile.tests
!include Makefile.mutatees
