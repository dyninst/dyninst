#
# $Id: Makefile,v 1.1 2008/06/30 19:40:38 legendre Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

NO_IMPLICIT_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 
TARGET = libstackwalk.so

# Now make any necessary architecture specific changes to variables:

LDFLAGS     += $(LIBDIR) -lcommon -ldl

SRCS = 	../src/linux-swk.C \
	../../common/src/Elf_X.C \
	../src/linux-ppc-swk.C \
	../src/ppc-swk.C \
	../src/symtab-swk.C \
	../src/linuxbsd-swk.C

ifdef LIBELF_INC
MYFLAGS += -I$(LIBELF_INC)
endif

ifdef LIBELF_LIB
MYFLAGS += -L$(LIBELF_LIB) -lelf
endif

LIBS	+=
CFLAGS  += -m32 $(MYFLAGS)
CXXFLAGS  += -m32 $(MYFLAGS)
AS		= as

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

# Now make any necessary architecture specific changes to variables:
VPATH += ../../common/src
