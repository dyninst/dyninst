#
# $Id: Makefile,v 1.18 2007/01/22 22:19:52 giri Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_LIBERTY = true
USES_SHM_SAMPLING = true
NO_IMPLICIT_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS = 		../src/init-sunos.C \
		../src/proc-solaris.C 

CXXFLAGS 	+= -DNO_MDL_PRINT
CFLAGS 		+= -DNO_MDL_PRINT

# Tradeoff: adding -g to TFLAGS aids gdb but uses gobs of disk space
#TFLAGS 		+= -g

SYSLIBS = -lelf -lsocket -lnsl -ldl -lrt
LIBS      =  -L$(LIBDWARF_LIB) -ldwarf

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.program.tmpl
