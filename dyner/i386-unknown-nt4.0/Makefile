#
# Makefile for dyninst test programs on Windows NT.
#
# $Id: Makefile,v 1.4 2002/06/03 18:58:31 pcroth Exp $
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

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

!include ../../nmake.program.tmpl
