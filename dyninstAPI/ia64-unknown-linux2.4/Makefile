#
# $Id: Makefile,v 1.17 2004/05/12 22:37:46 legendre Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

#DEBUG_FLAG = -g
NO_IMPLICIT_TEMPLATES = true
USES_DWARF_DEBUG = true
USES_UNWIND_LIB = true
USES_LIBELF = true

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
		../src/unix.C \
		../src/parseStab.C \
		../src/func-reloc.C \
		../src/LocalAlteration.C \
		../src/LocalAlteration-ia64.C \
		../src/FunctionExpansionRecord.C \
		../src/addLibraryLinux.C \
		../src/InstrucIter-ia64.C \
		../src/parseDwarf.C \
		../../common/src/linuxKludges.C \
		../../common/src/timing-linux.C \
		../src/ia64-template.s	\
		../src/syscall-linux.C

MYFLAGS		+=  -DUSES_DWARF_DEBUG -I$(LIBDWARFDIR) \
			   -I$(LIBUNWINDDIR)/include -I$(LIBELFDIR)/include \
            -I/usr/include/libelf

CXXFLAGS	+= $(MYFLAGS)
CFLAGS		+= $(MYFLAGS)
TFLAGS		+= $(MYFLAGS)

# Tradeoff: adding -g to TFLAGS aids gdb but uses gobs of disk space
#TFLAGS           += -g

SYSLIBS		= -ldl

LIBS		= -L$(LIBDWARFDIR)/lib -ldwarf -L$(LIBELFDIR)/bin
ifdef LIBELFSTATIC
LIBS    += -Wl,-static -lelf -Wl,-call_shared
else
LIBS    += -lelf
endif
LIBS    += -L$(LIBUNWINDDIR)/lib -Wl,-Bstatic -lunwind -Wl,-Bdynamic

AS		= /usr/bin/as

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
