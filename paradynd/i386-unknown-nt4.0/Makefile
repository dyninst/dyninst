#
# $Id: Makefile,v 1.16 2003/07/31 19:01:04 schendel Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

#USES_LIBERTY = true
USES_SHM_SAMPLING = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../nmake.config 

# Now make any necessary architecture specific changes to variables:

SRCS =		../src/pdwinnt.C \
		../src/Object-nt.C \
		../src/init-winnt.C \
		../src/proc-windows.C \
		../src/inst-winnt.C \
		../src/inst-x86.C \
		../src/func-reloc.C \
		../src/LocalAlteration.C \
		../src/LocalAlteration-x86.C \
		../src/FunctionExpansionRecord.C \
		../src/arch-x86.C \
		../src/arch-ia32.C \
		../../rtinst/src/RThwtimer-winnt.c \
		../../dyninstAPI/src/InstrucIter-x86.C

OBJS = 		pdwinnt.obj \
		Object-nt.obj \
		init-winnt.obj \
		proc-windows.obj \
		inst-winnt.obj \
		inst-x86.obj \
		func-reloc.obj \
		LocalAlteration.obj \
		LocalAlteration-x86.obj \
		FunctionExpansionRecord.obj \
		arch-x86.obj \
		arch-ia32.obj \
		RThwtimer-winnt.obj \
		InstrucIter-x86.obj

CXXFLAGS 	= $(CXXFLAGS) -DNO_MDL_PRINT 

#CXXFLAGS 	= $(CXXFLAGS) -DSAMPLEVALUE_DEBUG

SYSLIBS	        = $(SYSLIBS) advapi32.lib iphlpapi.lib psapi.lib


# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../nmake.module.tmpl

{../../rtinst/src/}.c.obj:
        $(CC) -c $(CFLAGS) $<

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../nmake.program.tmpl


