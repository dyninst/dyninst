#
# $Log: Makefile,v $
# Revision 1.1  2000/03/31 01:58:54  wylie
# Platform migration
#
# Revision 1.1  1997/10/15 19:47:31  tung
# Linux/X86 Port
#
# Revision 1.1  1996/10/18 23:33:19  mjrg
# Solaris/X86 port
#
#
#
# Define any symbols needed to invoke configuration changes in make.config

USES_FLEX = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

# none

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.program.tmpl

