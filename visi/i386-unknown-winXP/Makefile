#
# $Id: Makefile,v 1.2 2003/10/30 00:57:56 rchen Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

INSTANTIATE_TEMPLATES	= true
DO_INLINE_ALL		= true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../nmake.config 

#CXXFLAGS	=  $(CXXFLAGS) -DSAMPLEVALUE_DEBUG
# Now make any necessary architecture specific changes to variables:

CXXFLAGS    = $(CXXFLAGS) -MT

# none 

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../nmake.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../nmake.library.tmpl
