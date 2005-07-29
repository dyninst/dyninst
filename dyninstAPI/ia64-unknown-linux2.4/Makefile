#
# $Id: Makefile,v 1.23 2005/07/29 19:17:13 bernat Exp $
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

SRCS =	../src/linux.C \
		../src/linux-ia64.C \
		../src/linuxDL.C \
		../src/dynamiclinking.C \
		../src/linuxMT.C \
		../src/inst-linux.C \
		../src/inst-ia64.C \
		../src/arch-ia64.C \
		../src/Object-elf.C \
		../src/image-ia64.C \
		../src/unix.C \
		../src/parseStab.C \
		../src/addLibraryLinux.C \
		../src/InstrucIter-ia64.C \
		../src/parseDwarf.C \
		../../common/src/linuxKludges.C \
		../../common/src/timing-linux.C \
		../src/ia64-template.s \
		../src/syscall-linux.C

MYFLAGS		+= 	-DUSES_DWARF_DEBUG -I$(LIBDWARF_INC) \
				-I$(LIBUNWIND_INC) -I$(LIBELF_INC)

CXXFLAGS	+= $(MYFLAGS)
CFLAGS		+= $(MYFLAGS)
TFLAGS		+= $(MYFLAGS) 

SYSLIBS		= -ldl

LIBS		= -L$(LIBDWARF_LIB) -L$(LIBELF_LIB)
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

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
