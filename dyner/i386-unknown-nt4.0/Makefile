#
# Makefile for dyninst test programs on Windows NT.
#
# $Id: Makefile,v 1.5 2005/02/10 20:39:15 jaw Exp $
#

# Define any symbols needed to invoke configuration changes in nmake.config

TO_CORE=..\..
NO_OPT_FLAG=true
USES_TCLTK=true
USES_FLEX=true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

!include ../../nmake.config

# Now make any necessary architecture specific changes to variables:

#CFLAGS = $(CFLAGS) /Z7 /Ox
#CXXFLAGS = $(CXXFLAGS) /Z7 /Ox

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

!include ../nmake.module.tmpl

#CFLAGS = $(CFLAGS) -DWIN32_LEAN_AND_MEAN
#CXXFLAGS = $(CFLAGS) -DWIN32_LEAN_AND_MEAN
#CFLAGS = $(CFLAGS) -UERROR
#CXXFLAGS = $(CXXFLAGS) -UERROR
# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

!include ../../nmake.program.tmpl
