#
# $Log: Makefile,v $
# Revision 1.1  1997/08/18 01:34:18  buck
# Ported the Dyninst API to Windows NT.
#
# Revision 1.1.1.1  1997/06/11 17:38:11  buck
# Update Maryland repository with latest changes from Wisconsin.
#
# Revision 1.1  1997/05/23 23:02:09  mjrg
# Windows NT port
#
#
#
# Define any symbols needed to invoke configuration changes in make.config

#USES_LIBERTY = true
#USES_PVM = true

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

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../nmake.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../nmake.library.tmpl


