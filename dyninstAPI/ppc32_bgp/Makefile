#
# $Id: Makefile,v 1.4 2008/02/26 06:59:38 jaw Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

#USES_SHM_SAMPLING = true
#USES_PAPI = true
#DEBUG_FLAG = -g
AUTO_TEMPLATES=true
#NO_IMPLICIT_TEMPLATES = true
USES_BOOST = true

# For debugging.
#NO_OPT_FLAG = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS =          ../src/linux.C \
		../src/linux-power.C \
		../src/linuxDL.C \
		../src/dynamiclinking.C \
		../src/linuxMT.C \
		../src/inst-linux.C \
		../src/inst-power.C \
		../src/unix.C \
		../src/codegen-power.C \
		../src/image-power.C \
		../src/syscall-linux.C \
		../src/BPatch_memoryAccessAdapter.C
#		../src/addLibraryLinux.C \
#		../src/writeBackElf.C \
#		../src/saveSharedLibrary.C \

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
CXXFLAGS        += $(MYFLAGS)
CFLAGS          += $(MYFLAGS)
TFLAGS          += $(MYFLAGS)

# Tradeoff: adding -g to TFLAGS aids gdb but uses gobs of disk space
#TFLAGS		+= -g

SYSLIBS		= -ldl 
LIBS    += -lgcc

# AS		= /usr/bin/as

# no support for elf64_XXX() functions
# CXXFLAGS	+= -DUSES_ELF32_ONLY
# CFLAGS	+= -DUSES_ELF32_ONLY
# TFLAGS	+= -DUSES_ELF32_ONLY

# Add preprocessor and special link information for G++
G_PTHREAD	= -pthread
G_PTHREAD_LD	= -pthread

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
