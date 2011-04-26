#
# $Id: Makefile,v 1.1 2007/08/01 18:47:28 ssuen Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

NO_IMPLICIT_TEMPLATES = true
USES_DWARF_DEBUG = true
USES_LIBELF = true

STATIC_COMPS = true;

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.
include ../../make.config 

# Now make any necessary architecture specific changes to variables:
MYFLAGS         += -I../../common/src -I../../common/h
CXXFLAGS        += $(MYFLAGS)
CXXFLAGS    += -I/bgsys/drivers/ppcfloor/arch/include

CFLAGS          += $(MYFLAGS)
TFLAGS          += $(MYFLAGS)

SRCS = ../src/ppc-swk.C \
		 ../src/bg_firstparty.C \
		 ../src/firstparty.C \
		 ../../common/src/dyn_regs.C \
		 ../../common/src/addrtranslate.C \
		 ../../common/src/addrtranslate-sysv.C \
		 ../../common/src/addrtranslate-bluegenep.C \
       ../../common/src/Elf_X.C \
		 ../../common/src/bluegeneKludges.C \
		 ../../common/src/string-regex.C

ifdef LIBELF_INC
MYFLAGS += -I$(LIBELF_INC)
endif

ifdef LIBELF_LIB
LIBS += -L$(LIBELF_LIB)
endif
LIBS += -lelf

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

# Now make any necessary architecture specific changes to variables:
VPATH += ../../common/src
