#
# $Id: Makefile,v 1.6 2003/07/25 20:40:55 schendel Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_LIBERTY = true
USES_SHM_SAMPLING = true
USES_PVM = true
NO_IMPLICIT_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS =		../../dyninstAPI/src/solaris.C \
		../../dyninstAPI/src/solarisDL.C \
		../../dyninstAPI/src/sol_proc.C \
		../src/init-sunos.C \
		../../dyninstAPI/src/solarisMT.C \
		../src/proc-solaris.C \
		../../dyninstAPI/src/inst-sparc.C \
		../../dyninstAPI/src/inst-sparc-solaris.C \
		../../dyninstAPI/src/inst-sunos.C \
		../../dyninstAPI/src/Object-elf.C \
		../../dyninstAPI/src/tramp-sparc.s \
		../../dyninstAPI/src/unix.C \
		../../dyninstAPI/src/FunctionExpansionRecord.C \
		../../dyninstAPI/src/LocalAlteration.C \
		../../dyninstAPI/src/func-reloc.C \
		../../dyninstAPI/src/LocalAlteration-Sparc.C

CXXFLAGS 	+= -DNO_MDL_PRINT -DUSES_DYNAMIC_INF_HEAP
CFLAGS 		+= -DNO_MDL_PRINT -DUSES_DYNAMIC_INF_HEAP

# Tradeoff: adding -g to TFLAGS aids gdb but uses gobs of disk space
#TFLAGS 		+= -g

SYSLIBS = -lelf -lsocket -lnsl -ldl

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.program.tmpl
