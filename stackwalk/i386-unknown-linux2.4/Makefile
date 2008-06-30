#
# $Id: Makefile,v 1.1 2008/06/30 19:40:35 legendre Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

NO_IMPLICIT_TEMPLATES = true
TARGET = libstackwalk.so

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

LDFLAGS     += $(LIBDIR) -lcommon -ldl

SRCS = 	../src/linux-swk.C \
			../src/linux-x86-swk.C \
			../src/x86-swk.C \
			../src/symtab-swk.C

LIBS	+=
CFLAGS  += -fno-omit-frame-pointer -Dcap_sw_catchfaults -O2 -fno-inline
CXXFLAGS  += -fno-omit-frame-pointer -Dcap_sw_catchfaults -O2 -fno-inline
AS		= as

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
