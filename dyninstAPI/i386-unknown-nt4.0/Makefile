#
# $Id: Makefile,v 1.4 2000/10/17 17:42:11 schendel Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../nmake.config 

# Now make any necessary architecture specific changes to variables:

SRCS =		../src/pdwinnt.C \
		../src/inst-winnt.C \
		../src/inst-x86.C \
		../src/arch-x86.C 

OBJS = 		pdwinnt.obj \
		inst-winnt.obj \
		inst-x86.obj \
		arch-x86.obj


CXXFLAGS 	= $(CXXFLAGS) -DNO_MDL_PRINT
CXXFLAGS 	= $(CXXFLAGS) -GX  #for exception handling

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../nmake.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../nmake.library.tmpl


# DO NOT DELETE
