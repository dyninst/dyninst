#
# $Id: Makefile,v 1.7 2004/11/30 21:10:53 tlmiller Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_LIBERTY = true
USES_LIBELF = true
USES_UNWIND_LIB = true
USES_SHM_SAMPLING = true
NO_IMPLICIT_TEMPLATES = true

DEBUG_FLAG = -g
NO_OPT_FLAG = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

VPATH	+= ../../rtinst/h:../../rtinst/src

SRCS 	= \
		../src/init-linux.C \
		../src/proc-linux.C \
		../../rtinst/src/RThwtimer-linux.c \
		../../dyninstAPI/src/linux.C \
		../../dyninstAPI/src/linux-ia64.C \
		../../dyninstAPI/src/linuxDL.C \
		../../dyninstAPI/src/linuxMT.C \
		../../dyninstAPI/src/syscall-linux.C \
		../../dyninstAPI/src/inst-linux.C \
		../../dyninstAPI/src/unix.C  \
		../../dyninstAPI/src/Object-elf.C \
		../../dyninstAPI/src/parseStab.C \
		../../dyninstAPI/src/parseDwarf.C \
		../../dyninstAPI/src/dynamiclinking.C \
		../../dyninstAPI/src/FunctionExpansionRecord.C \
		../../dyninstAPI/src/inst-ia64.C \
		../../dyninstAPI/src/arch-ia64.C \
		../../dyninstAPI/src/InstrucIter-ia64.C \
		../../dyninstAPI/src/ia64-template.s

# The different debug flags.
#MYFLAGS	+= -DPDYN_DEBUG
#MYFLAGS	+= -DATTACH_DETACH_DEBUG -DINFERIOR_RPC_DEBUG
#MYFLAGS	+= -DPTRACEDEBUG
#MYFLAGS	+= -DPTRACEDEBUG_EXCESSIVE
#MYFLAGS	+= -DSIGNAL_DEBUG
#MYFLAGS	+= -DINST_TRAP_DEBUG
#MYFLAGS	+= -DLD_DEBUG
#MYFLAGS	+= -DSAMPLEVALUE_DEBUG
#MYFLAGS	+= -DAGGREGATE_DEBUG

MYFLAGS		+=	-DUSES_DWARF_DEBUG -I$(LIBDWARF_INC) -I$(LIBUNWIND_INC) -I$(LIBELF_INC)
MYFLAGS		+=	-DNO_MDL_PRINT
MYFLAGS		+= 	-I../../dyninstAPI/src

CFLAGS		+= $(MYFLAGS)
CXXFLAGS	+= $(MYFLAGS)
TFLAGS		+= $(MYFLAGS)

SYSLIBS = -ldl

LIBS	=	-L$(LIBDWARF_LIB) -L$(LIBELF_LIB)
ifdef LIBDWARFSTATIC
LIBS	+=	-Wl,-Bstatic -ldwarf -Wl,-Bdynamic
else
LIBS	+= 	-ldwarf
endif
ifdef LIBELFSTATIC
LIBS	+=	-Wl,-static -lelf -Wl,-call_shared
else
LIBS	+=	-lelf
endif
LIBS	+= -L$(LIBUNWIND_LIB) -lunwind-ptrace -lunwind -lunwind-ia64

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.program.tmpl
