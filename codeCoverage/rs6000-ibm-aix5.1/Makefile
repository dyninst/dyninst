INSTANTIATE_TEMPLATES   = true
DO_INLINE_ALL           = true
NO_OPT_FLAG = true

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

# Override default of gcc -- need g++ on AIX
LD = g++ 
# On AIX all symbols must be resolved at link time, even for a library.
# We need the cplus.demangle function from libiberty.
LDFLAGS += -liberty 

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

TARGET	= dyncov

CXX	= g++

include ../../make.program.tmpl
