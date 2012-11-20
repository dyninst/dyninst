#
# $Id: Makefile,v 1.10 2008/05/27 20:44:56 giri Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

#USES_LIBERTY = true
#NO_OPT_FLAG = true
# DEBUG_FLAG = -g
NO_IMPLICIT_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS =		../src/Object-elf.C \
		../src/Archive.C \
		../src/Archive-elf.C \
		../src/parseDwarf.C \
		../src/parseStab.C \
		../src/emitElf.C \
		../src/emitElfStatic.C \
		../src/emitElfStatic-stub.C \
		../src/LinkMap.C \
		../src/relocationEntry-elf-sparc.C

# VG(12/02/01) Using -g here seems a bad idea since it is passed from
# make.confing any way and it is actually configurable there via
# DEBUG_FLAG....

CFLAGS		+= -DNO_MDL_PRINT -I/usr/local/stow/bind-8.2.2.p5/include/ $(MYFLAGS)
#CFLAGS		+= -DNO_MDL_PRINT -O0 -I/usr/local/stow/bind-8.2.2.p5/include/ 
CXXFLAGS	+= -DNO_MDL_PRINT -DUSES_ELF32_ONLY  -I/usr/local/stow/bind-8.2.2.p5/include/ -DUSES_DWARF_DEBUG $(MYFLAGS)

#CXXFLAGS	+= -DNO_MDL_PRINT -O0 -DUSES_ELF32_ONLY  -I/usr/local/stow/bind-8.2.2.p5/include/
TFLAGS		+= -DUSES_ELF32_ONLY

# Tradeoff: adding -g to TFLAGS aids gdb but uses gobs of disk space
#TFLAGS 		+= -g

SYSLIBS = -lsocket -lnsl -ldl -lrt

# Add preprocessor and special link information for G++

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

# Now make any necessary architecture specific changes to variables:
VPATH += ../../common/src
