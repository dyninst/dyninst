#
# $Log: Makefile,v $
# Revision 1.3  2006/04/13 23:06:06  legendre
# Fixes to get visis building in new Windows build system
#
# Revision 1.2  2005/12/19 19:44:15  pack
# MRNet replaces xdr for frontend to daemon communication.  New igen  - - -
# MRNet changes
#
# Revision 1.1.1.1  2004/11/05 22:17:52  darnold
# start
#
# Revision 1.1.1.1  2004/10/18 20:35:55  darnold
# Manual Branch for MRNet Integration based on Release_4.1
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
SYSLIBS		+= -ldl -lSM -lICE -lXpm
#SYSLIBS		= -lsocket -lnsl

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../../make.program.tmpl
