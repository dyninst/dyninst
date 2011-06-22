#
# $Id: Makefile,v 1.41 2008/09/04 21:06:06 bill Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

#USES_SHM_SAMPLING = true
#USES_PAPI = true
#DEBUG_FLAG = -g
#AUTO_TEMPLATES= true
NO_IMPLICIT_TEMPLATES = true
USES_DWARF_DEBUG = true
USES_LIBELF = true
USES_BOOST = true

# For debugging.
#NO_OPT_FLAG = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS =          ../src/freebsd.C \
		../src/freebsd-x86.C \
		../src/RegisterConversion-x86.C \
		../src/Relocation/Widgets/CFWidget-x86.C \
		../src/dynamiclinking.C \
                ../src/inst-x86.C \
		../src/emit-x86.C \
		../src/codegen-x86.C \
                ../src/unix.C \
		../src/stackwalk-x86.C \
		../src/parse-x86.C \
		../src/frameChecker.C \
		../src/IAPI_to_AST.C \
		../src/BPatch_memoryAccessAdapter.C \

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
MYFLAGS         += -DUSES_DWARF_DEBUG -I$(LIBDWARF_INC)
MYFLAGS         += -I$(LIBELF_INC)

CXXFLAGS        += $(MYFLAGS)
CFLAGS          += $(MYFLAGS)
TFLAGS          += $(MYFLAGS)
LDFLAGS 	+= -Wl,-rpath,$(LIBDWARF_LIB)

# Tradeoff: adding -g to TFLAGS aids gdb but uses gobs of disk space
#TFLAGS		+= -g

SYSLIBS		=

LIBS      +=  -L$(LIBDWARF_LIB) -L$(LIBELF_LIB)
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
LIBS    += -lgcc -pthread

# InstructionAPI is x86-only at this point
LIBS    += -linstructionAPI

AS		= /usr/bin/as

# no support for elf64_XXX() functions
CXXFLAGS	+= -DUSES_ELF32_ONLY
CFLAGS		+= -DUSES_ELF32_ONLY
TFLAGS		+= -DUSES_ELF32_ONLY

# Add preprocessor and special link information for G++
G_PTHREAD	= -pthread
G_PTHREAD_LD	= -pthread

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
