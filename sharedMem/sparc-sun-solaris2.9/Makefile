#
# $Id: Makefile,v 1.2 2004/07/15 21:24:03 jaw Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture-specific changes to variables:
USES_NATIVE_CC = true
NATIVE_CC = cc
ifneq (,$(findstring WorkShop,$(shell $(NATIVE_CC) -V 2>&1)))
USES_NATIVE_CC = true
endif
ifneq (,$(findstring Sun C,$(shell $(NATIVE_CC) -V 2>&1)))
USES_NATIVE_CC = true
endif


ifdef USES_NATIVE_CC
CC		= $(NATIVE_CC)
LD		= $(NATIVE_CC)
# OVERRIDE CFLAGS
CFLAGS		+= $(IFLAGS) $(UNIFIED_DEF) $(COMMON_WARNINGS)
CFLAGS		+= -Kpic -O
LDFLAGS		+= -dy -G -z text
else
CC		= $(GCC)
LD		= $(GCC)
CFLAGS		+= -fpic -O2
LDFLAGS		+= -shared 
endif

VPATH        = ../src
#:../../dyninstAPI_RT/src # ccw 18 apr 2002 : SPLIT

SRCS	     += ../src/RTshared-unix.c

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

