#
# $Id: Makefile,v 1.2 1998/04/01 02:36:32 wylie Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

INSTANTIATE_TEMPLATES   = true
DO_INLINE_ALL           = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

!include ../../nmake.config 

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

!include ../nmake.module.tmpl

# Now make any necessary architecture specific changes to variables:

#CXXFLAGS		= $(CXXFLAGS)

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

!include ../../nmake.library.tmpl
