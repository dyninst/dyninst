#
# $Log: Makefile,v $
# Revision 1.1  2002/02/11 22:01:30  tlmiller
# introduce IA-64 port  - - - - - - - - - - - - - - - - - - - - - - - - - -
# the IA-64 Makefile(s)
#
# Revision 1.2  2000/07/18 17:02:25  schendel
# add optional flag to turn on printing of sample value trace lines
#
# Revision 1.1  2000/03/31 01:59:06  wylie
# Platform migration
#
# Revision 1.1  1997/10/15 21:10:15  tung
# Linux/X86 Port
#
# Revision 1.1  1996/10/18 23:35:12  mjrg
# Solaris/X86 port
#
#
# Define any symbols needed to invoke configuration changes in make.config

USES_TCLTK = true
USES_FLEX = true
USES_X11 = true
NO_IMPLICIT_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:
CFLAGS		+=  -DSHG_ADD_ON_EVAL
CXXFLAGS	+=  -DSHG_ADD_ON_EVAL
#CXXFLAGS	+=  -DSAMPLEVALUE_DEBUG
TFLAGS		+=  -DSHG_ADD_ON_EVAL
SYSLIBS		+=  -ldl
#SYSLIBS		+= -lresolv -lsocket -lnsl

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Now make any necessary architecture specific changes to variables:

# none to make

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.program.tmpl
