#
# $Log: Makefile,v $
# Revision 1.1  2000/03/31 02:00:10  wylie
# Platform migration
#
# Revision 1.1  1997/10/15 21:19:53  tung
# Linux/X86 Port
#
# Revision 1.1  1996/10/18 23:43:11  mjrg
# Solaris/X86 port
#
#
# Define any symbols needed to invoke configuration changes in make.config

USES_TCLTK	= true
USES_X11	= true
TO_CORE		= ../../..
NO_IMPLICIT_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:
# this two are a temporary kludge; barchart should have a "src" dir
VPATH		+= ..
CXXFLAGS	+= $(TCLFRIENDLYWARNINGS)
TFLAGS		+= $(TCLFRIENDLYWARNINGS)
CFLAGS		+= $(TCLFRIENDLYWARNINGS)
SYSLIBS		+= -ldl
#SYSLIBS		= -lsocket -lnsl

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../../make.program.tmpl
