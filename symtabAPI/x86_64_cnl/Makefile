#
# $Id: Makefile,v 1.8 2008/05/27 20:44:58 giri Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

#USES_SHM_SAMPLING = true
#USES_PAPI = true
#DEBUG_FLAG = -g
NO_IMPLICIT_TEMPLATES = true
USES_DWARF_DEBUG = true
USES_LIBELF = true

# For debugging.
NO_OPT_FLAG = true

STATIC_COMPS = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS =          ../src/Object-elf.C \
		../../common/src/Elf_X.C \
		../src/Archive.C \
		../src/Archive-elf.C\
		../src/parseDwarf.C \
		../src/parseStab.C \
		../src/emitElf.C \
		../src/emitElf-64.C \
		../src/emitElfStatic.C \
		../src/emitElfStatic-stub.C \
		../src/LinkMap.C \
		../src/relocationEntry-elf-x86.C


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

CXXFLAGS        += $(MYFLAGS)
CFLAGS          += $(MYFLAGS)
TFLAGS          += $(MYFLAGS)

# Tradeoff: adding -g to TFLAGS aids gdb but uses gobs of disk space
#TFLAGS		+= -g

LIBS	  +=  -L$(LIBDWARF_LIB) -L$(LIBELF_LIB)
LIBS += -ldwarf
LIBS += -lelf

AS		= /usr/bin/as

# Add preprocessor and special link information for G++
G_PTHREAD       = -pthread

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

# Now make any necessary architecture specific changes to variables:
VPATH += ../../common/src
