#
# $Id: Makefile,v 1.1 2001/06/04 16:24:20 schendel Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

INSTANTIATE_TEMPLATES	= true
DO_INLINE_ALL		= true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

#CXXFLAGS        +=  -DSAMPLEVALUE_DEBUG
# Now make any necessary architecture specific changes to variables:

# none 

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
