#
# $Id: Makefile,v 1.1 2005/02/24 10:14:03 rchen Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:
LD		= $(GCC)
LDFLAGS         += -shared -ldl 

# The GNU headers for glibc conflict with RTposix.h
# CFLAGS		+= -D_SVID_SOURCE

# if we want to produce a shared object...
CFLAGS		+= -fPIC
CFLAGS          += -I. -I../..

VPATH        = ../src
#:../../dyninstAPI_RT/src

SRCS	     += ../src/RTshared-unix.c 

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

