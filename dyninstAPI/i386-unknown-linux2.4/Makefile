#
# $Id: Makefile,v 1.24 2005/02/07 16:58:19 gquinn Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

#USES_SHM_SAMPLING = true
#USES_PAPI = true
#DEBUG_FLAG = -g
NO_IMPLICIT_TEMPLATES = true
USES_DWARF_DEBUG = true
USES_LIBELF = true

# For debugging.
#NO_OPT_FLAG = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS =          ../src/linux.C \
		../src/linux-x86.C \
		../src/linuxDL.C \
		../src/dynamiclinking.C \
		../src/linuxMT.C \
                ../src/inst-linux.C \
                ../src/inst-x86.C \
                ../src/arch-x86.C \
		../src/Object-elf.C \
                ../src/unix.C \
		../src/parseStab.C \
		../src/InstrucIter-x86.C \
		../src/func-reloc.C \
	        ../src/LocalAlteration.C \
	        ../src/LocalAlteration-x86.C \
		../src/FunctionExpansionRecord.C \
		../src/addLibraryLinux.C \
		../src/writeBackElf.C \
		../src/saveSharedLibrary.C \
		../src/parseDwarf.C \
		../../common/src/linuxKludges.C \
		../../common/src/timing-linux.C \
		../src/syscall-linux.C

#MYFLAGS	+= -DNO_MDL_PRINT
#MYFLAGS	+= -DLD_DEBUG
#MYFLAGS	+= -DATTACH_DETACH_DEBUG
#MYFLAGS	+= -DPTRACEDEBUG
#MYFLAGS	+= -DPTRACEDEBUG_EXCESSIVE
#MYFLAGS	+= -DPTRACEDEBUG_ALWAYS
#MYFLAGS	+= -DINST_TRAP_DEBUG
#MYFLAGS	+= -DSIGNAL_DEBUG
#MYFLAGS	+= -DASTDEBUG
#MYFLAGS	+= -DINFERIOR_RPC_DEBUG
MYFLAGS		+= -DUSES_DWARF_DEBUG -I$(LIBDWARF_INC)
MYFLAGS     += -I$(LIBELF_INC) 
MYFLAGS		+= -I/usr/include/libelf

CXXFLAGS        += $(MYFLAGS)
CFLAGS          += $(MYFLAGS)
TFLAGS          += $(MYFLAGS)

# Tradeoff: adding -g to TFLAGS aids gdb but uses gobs of disk space
#TFLAGS		+= -g

SYSLIBS		= -ldl 

LIBS	  +=  -L$(LIBDWARF_LIB) -L$(LIBELF_LIB)
ifdef LIBDWARFSTATIC
LIBS    += -Wl,-static -ldwarf -Wl,-call_shared
else
LIBS    += -ldwarf
endif
ifdef LIBELFSTATIC
LIBS    += -Wl,-static -lelf -Wl,-call_shared
else
LIBS    += -lelf
endif
LIBS    += -lgcc

AS		= /usr/bin/as

# no support for elf64_XXX() functions
CXXFLAGS	+= -DUSES_ELF32_ONLY
CFLAGS		+= -DUSES_ELF32_ONLY
TFLAGS		+= -DUSES_ELF32_ONLY

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
