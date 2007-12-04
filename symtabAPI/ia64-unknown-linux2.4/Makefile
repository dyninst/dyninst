#
# $Id: Makefile,v 1.7 2007/12/04 18:05:34 legendre Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

DEBUG_FLAG = -g
NO_IMPLICIT_TEMPLATES = true
USES_DWARF_DEBUG = true
USES_LIBELF = true
USES_UNWIND_LIB = true

# Turns off optimization, and enables debugging.
# NO_OPT_FLAG = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.
include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS =		../src/Object-elf.C \
		../src/parseDwarf.C \
		../src/parseStab.C \
                ../src/emitElf.C \
                ../src/emitElf-64.C \
		../src/addrtranslate.C \
		../src/addrtranslate-linux.C \
		../src/addrtranslate-sysv.C

MYFLAGS		+= 	-DUSES_DWARF_DEBUG -I$(LIBDWARF_INC) \
				-I$(LIBUNWIND_INC) -I$(LIBELF_INC)

CXXFLAGS	+= $(MYFLAGS)
CFLAGS		+= $(MYFLAGS)
TFLAGS		+= $(MYFLAGS) 

SYSLIBS		= -ldl

LIBS		= -L$(LIBDWARF_LIB) -L$(LIBELF_LIB) -R$(LIBDWARF_LIB):$(LIBELF_LIB)
ifdef LIBDWARFSTATIC
LIBS    += -Wl,-Bstatic -ldwarf -Wl,-Bdynamic
else
LIBS    += -ldwarf
endif
ifdef LIBELFSTATIC
LIBS    += -Wl,-Bstatic -lelf -Wl,-Bdynamic
else
LIBS    += -lelf
endif
LIBS    += -L$(LIBUNWIND_LIB) -lunwind-ptrace -lunwind -lunwind-ia64

# Add preprocessor and special link information for G++
G_PTHREAD       = -pthread
G_PTHREAD_LD	= -pthread

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
