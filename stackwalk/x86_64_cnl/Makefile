#
# $Id: Makefile,v 1.1 2008/06/30 19:40:57 legendre Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

NO_IMPLICIT_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

STATIC_COMPS = true

include ../../make.config 

TARGET = libstackwalk.a

LDFLAGS     += $(LIBDIR) -lcommon

SRCS = 	../src/linux-swk.C \
	../src/linux-x86-swk.C \
	../src/x86-swk.C \
	../src/symtab-swk.C \
	../src/dbginfo-stepper.C \
	../src/x86-wanderer.C

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

# Now make any necessary architecture specific changes to variables:

ifdef LIBELF_INC
MYFLAGS += -I$(LIBELF_INC)
endif
ifdef LIBDWARF_INC
MYFLAGS += -I$(LIBDWARF_INC)
endif

ifdef LIBELF_LIB
LDFLAGS += -L$(LIBELF_LIB) -lelf
endif
ifdef LIBDWARF_LIB
LDFLAGS += -L$(LIBDWARF_LIB) -ldwarf
endif

LIBS	+= 
CFLAGS  += -fno-omit-frame-pointer -Dcap_sw_catchfaults $(MYFLAGS)
CXXFLAGS  += -fno-omit-frame-pointer -Dcap_sw_catchfaults $(MYFLAGS)
AS		= as

# Now make any necessary architecture specific changes to variables:
VPATH += ../../common/src
