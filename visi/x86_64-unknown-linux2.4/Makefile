#
# $Log: Makefile,v $
# Revision 1.1  2005/02/24 10:14:31  rchen
# Introduction of the x86_64-unknown-linux2.4 platform.  - - - - - - - - -
# Platform directory structure creation.
#
# Revision 1.2  2000/07/18 17:02:42  schendel
# add optional flag to turn on printing of sample value trace lines
#
# Revision 1.1  2000/03/31 01:59:59  wylie
# Platform migration
#
# Revision 1.1  1997/10/15 20:26:17  tung
# Linux/X86 Port
#
# Revision 1.1  1996/10/18 23:42:40  mjrg
# Solaris/X86 port
#
#
#
# Define any symbols needed to invoke configuration changes in make.config

INSTANTIATE_TEMPLATES	= true
DO_INLINE_ALL		= true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:
#CXXFLAGS	+=  -DSAMPLEVALUE_DEBUG

# none 

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
