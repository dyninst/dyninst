#
# $Id: Makefile,v 1.2 2004/07/28 07:24:48 jaw Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

INSTANTIATE_TEMPLATES   = true
DO_INLINE_ALL           = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

CFLAGS += 
TFLAGS +=
CXXFLAGS +=
#CXXFLAGS        +=  -DSAMPLEVALUE_DEBUG

ifdef GCC_2_95
CXXFLAGS	+= -fno-exceptions
TFLAGS		+= -fno-exceptions
endif

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

SRCS += ../src/templates-X.C

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

templates-X.o: ../src/templates-X.C
	$(CXX) $(TFLAGS) -c ../src/templates-X.C
