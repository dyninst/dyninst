#
# $Id: Makefile,v 1.1 2003/03/31 17:45:56 buck Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_TCLTK = true
USES_FLEX = true
USES_X11 = true
NO_IMPLICIT_TEMPLATES = true
TO_CORE = ../..

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:
CFLAGS 		+=  -DSHG_ADD_ON_EVAL
CXXFLAGS 	+=  -DSHG_ADD_ON_EVAL 
#CXXFLAGS        +=  -DSAMPLEVALUE_DEBUG
TFLAGS 		+=  -DSHG_ADD_ON_EVAL

TFLAGS		+= -fno-inline-functions

TEMP_INLINE_DEFINES =
#ALL_INLINE_DEFINES = 

# Tradoff: adding -g to TFLAGS aids gdb but uses up gobs of disk space
#TFLAGS 		+=  -g

SYSLIBS		+= -lstdc++ 
# Override these defaults by setting LDTCLFLAGS in top level make.config.local
# Recommended settig for AIX 4.3.3 w/IBM RPM supplied tcl-8.3.3 is:
# LDTCLFLAGS := -Wl,-brtl,-bbigtoc
LDTCLFLAGS ?= -Xlinker -brtl -Xlinker -bbigtoc
LDFLAGS += $(LDTCLFLAGS)

ifdef GCC_2_95
CXXFLAGS	+= -fno-exceptions
TFLAGS		+= -fno-exceptions
endif

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Now make any necessary architecture specific changes to variables:

# none to make

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include $(TO_CORE)/make.program.tmpl
