#
# $Log: Makefile,v $
# Revision 1.1  2000/03/31 02:00:24  wylie
# Platform migration
#
# Revision 1.1  1997/10/15 21:20:41  tung
# Linux/X86 Port
#
# Revision 1.1  1996/10/18 23:43:18  mjrg
# Solaris/X86 port
#
#
#

USES_TCLTK	= true
USES_X11	= true
TO_CORE		= ../../..

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../../make.config 

# Now make any necessary architecture specific changes to variables:
# this two are a temporary kludge; barchart should have a "src" dir
VPATH		+= ..
CXXFLAGS	+= -I.. 
TFLAGS		+= -I.. 
CFLAGS		+= -I.. 
SYSLIBS 	+= -ldl
#SYSLIBS		= -lsocket -lnsl

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../../make.program.tmpl
