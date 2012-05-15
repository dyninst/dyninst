#
# $Id: Makefile,v 1.1 2008/06/30 19:40:37 legendre Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

NO_IMPLICIT_TEMPLATES = true
TARGET = libstackwalk.a

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

LDFLAGS     += $(LIBDIR) -lcommon -ldl
CXXFLAGS    += -I/bgsys/drivers/ppcfloor/arch/include

SRCS =         ../src/bluegene-swk.C \
               ../../common/src/Elf_X.C \
               ../src/bluegenep-swk.C \
               ../src/ppc-swk.C \
               ../src/symtab-swk.C

STATIC_COMPS = true

LIBS   +=
CFLAGS  += -m32 $(MYFLAGS)
CXXFLAGS += -m32 $(MYFLAGS)
AS             = as

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

# Now make any necessary architecture specific changes to variables:
VPATH += ../../common/src
