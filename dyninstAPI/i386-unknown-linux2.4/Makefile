#
# $Id: Makefile,v 1.11 2002/06/14 21:50:11 tlmiller Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

#USES_SHM_SAMPLING = true
DEBUG_FLAG = -g -gstabs+
NO_IMPLICIT_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS =          ../src/linux.C \
		../src/linux-x86.C \
		../src/linuxDL.C \
                ../src/inst-linux.C \
                ../src/inst-x86.C \
                ../src/arch-x86.C \
                ../src/arch-ia32.C \
		../src/Object-elf.C \
                ../src/unix.C \
		../src/parseStab.C \
		../src/InstrucIter-x86.C \
		../src/func-reloc.C \
	        ../src/LocalAlteration.C \
	        ../src/LocalAlteration-x86.C \
		../src/FunctionExpansionRecord.C \
		../src/addLibraryLinux.C \
		../../common/src/linuxKludges.C \
		../../common/src/timing-linux.C


MYFLAGS          += -I/usr/include/libelf
#MYFLAGS          += -DNO_MDL_PRINT
#MYFLAGS		 += -DLD_DEBUG
#MYFLAGS          += -DATTACH_DETACH_DEBUG
#MYFLAGS		 += -DPTRACEDEBUG
#MYFLAGS		 += -DPTRACEDEBUG_EXCESSIVE
#MYFLAGS		 += -DPTRACEDEBUG_ALWAYS
#MYFLAGS		 += -DINST_TRAP_DEBUG
#MYFLAGS		 += -DSIGNAL_DEBUG
#MYFLAGS		 += -DASTDEBUG
#MYFLAGS		 += -DINFERIOR_RPC_DEBUG

CXXFLAGS         += $(MYFLAGS)
CFLAGS           += $(MYFLAGS)
TFLAGS           += $(MYFLAGS)

# Tradeoff: adding -g to TFLAGS aids gdb but uses gobs of disk space
#TFLAGS                 += -g

SYSLIBS = -lelf -ldl

AS		= /usr/bin/as

# no support for elf64_XXX() functions
CXXFLAGS	+= -DUSES_ELF32_ONLY
CFLAGS		+= -DUSES_ELF32_ONLY
TFLAGS		+= -DUSES_ELF32_ONLY

export NO_OPT_FLAG=foobar

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
