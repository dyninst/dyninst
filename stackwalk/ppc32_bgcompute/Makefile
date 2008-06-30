#
# $Id: Makefile,v 1.1 2008/06/30 19:40:36 legendre Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

TARGET = libstackwalk.a

NO_IMPLICIT_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

# LDFLAGS     += $(LIBDIR) -lcommon -ldl

SRCS = 	../src/firstparty.C \
			../src/ppc-swk.C 

STATIC_LIBRARY = true

LIBS	+=
CFLAGS  +=
AS		= as

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
