#
# $Id: Makefile,v 1.2 1998/04/01 02:51:56 wylie Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_FLEX = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../nmake.config 

# Now make any necessary architecture specific changes to variables:
# kluge for bison.simple and alloca
CXXFLAGS = $(CXXFLAGS)
# none

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../nmake.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../nmake.program.tmpl

