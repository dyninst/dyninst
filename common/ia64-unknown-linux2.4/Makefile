#
# $Id: Makefile,v 1.3 2007/02/21 22:37:50 giri Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

DEBUG_FLAG = -g
NO_IMPLICIT_TEMPLATES = true
USES_DWARF_DEBUG = true
USES_LIBELF = true
USES_UNWIND_LIB = true

# Turns off optimization, and enables debugging.
# NO_OPT_FLAG = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.
include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS =		../../common/src/linuxKludges.C \
		../../common/src/timing-linux.C

CXXFLAGS	+= $(MYFLAGS)
CFLAGS		+= $(MYFLAGS)
TFLAGS		+= $(MYFLAGS) 

SYSLIBS		= -ldl

# Add preprocessor and special link information for G++
G_PTHREAD       = -pthread
G_PTHREAD_LD	= -pthread

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
