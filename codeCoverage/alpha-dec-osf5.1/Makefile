# Define any symbols needed to invoke configuration changes in make.config

NO_IMPLICIT_TEMPLATES	 = true
NO_OPT_FLAG		 = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS =		 

CFLAGS += -g
TFLAGS +=
CXXFLAGS += -g

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.program.tmpl

LD = g++
LIBS += -lmld -liberty
