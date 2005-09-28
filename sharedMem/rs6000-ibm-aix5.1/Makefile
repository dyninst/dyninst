#
# $Id: Makefile,v 1.2 2005/09/28 17:02:38 bernat Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

#DEBUG_FLAG = -g

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

VPATH   = ../src
#:../../dyninstAPI_RT/src

SRCS	= ../src/RTshared-unix.c 

LD              = $(LINKER)
LDFLAGS		+= -bnoentry -bnoobjreorder -bexpall -lc 

LDFLAGS +=  `$(GCC) -print-libgcc-file-name`

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

