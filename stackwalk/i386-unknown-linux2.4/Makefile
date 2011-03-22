#
# $Id: Makefile,v 1.1 2008/06/30 19:40:35 legendre Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

NO_IMPLICIT_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

# Now make any necessary architecture specific changes to variables:
include ../make.module.tmpl

LDFLAGS     += $(LIBDIR) -lcommon -ldl

SRCS += 	../src/linux-swk.C \
			../src/linuxbsd-x86-swk.C \
			../src/x86-swk.C \
			../src/symtab-swk.C \
			../src/dbginfo-stepper.C \
			../src/x86-wanderer.C \
	                ../src/analysis_stepper.C \
			../src/linuxbsd-swk.C \
			../src/linux-x86-swk.C


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
MYFLAGS += -I../../parseAPI/h -I../../symtabAPI/h -I../../symEval/h
CFLAGS  += -fno-omit-frame-pointer -Dcap_sw_catchfaults -fno-inline $(MYFLAGS)
CXXFLAGS  += -fno-omit-frame-pointer -Dcap_sw_catchfaults -fno-inline $(MYFLAGS)
AS		= as

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

libstackwalk.a: $(OBJS)
	ar rv libstackwalk.a $^
