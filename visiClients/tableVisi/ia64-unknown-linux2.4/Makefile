#
# $Log: Makefile,v $
# Revision 1.1  2002/02/11 22:01:42  tlmiller
# introduce IA-64 port  - - - - - - - - - - - - - - - - - - - - - - - - - -
# the IA-64 Makefile(s)
#
# Revision 1.1  2000/03/31 02:00:31  wylie
# Platform migration
#
# Revision 1.1  1997/10/15 21:21:35  tung
# Linux/X86 Port
#
# Revision 1.1  1996/10/18 23:43:24  mjrg
# Solaris/X86 port
#
#
#

# Define any symbols needed to invoke configuration changes in make.config

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

TO_CORE		= ../../..
USES_TCLTK	= true
USES_X11        = true
NO_IMPLICIT_TEMPLATES = true

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:

SYSLIBS 	+= -ldl
#SYSLIBS 	+= -lsocket -lnsl

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include $(TO_CORE)/make.program.tmpl
