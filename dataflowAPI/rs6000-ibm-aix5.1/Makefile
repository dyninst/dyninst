# Define any symbols needed to invoke configuration changes in make.config

DEBUG_FLAG = -g
AUTO_TEMPLATES= true

# For debugging.
#NO_OPT_FLAG = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

CXXFLAGS        += $(MYFLAGS)
CFLAGS          += $(MYFLAGS)

LIBS    += -lgcc -linstructionAPI -lparseAPI

AS		= /usr/bin/as

TARGET = libdataflowAPI.a
# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
