#
# $Log: Makefile,v $
# Revision 1.1  2002/02/11 22:01:45  tlmiller
# introduce IA-64 port  - - - - - - - - - - - - - - - - - - - - - - - - - -
# the IA-64 Makefile(s)
#
# Revision 1.1  2000/03/31 02:00:43  wylie
# Platform migration
#
# Revision 1.1  1997/10/15 21:22:46  tung
# Linux/X86 Port
#
# Revision 1.1  1997/05/13 20:55:36  naim
# Adding Terrain to rs6000 and x86 platforms - naim
#
# Revision 1.1  1997/05/12 20:16:37  naim
# Adding "Terrain" visualization to paradyn (commited by naim, done by tung).
#
#
# Define any symbols needed to invoke configuration changes in make.config

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
