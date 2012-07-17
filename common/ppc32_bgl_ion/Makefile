#
# $Id: Makefile,v 1.1 2007/08/01 18:47:28 ssuen Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

NO_IMPLICIT_TEMPLATES = true

STATIC_COMPS = true;

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.
include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS =         ../../common/src/linuxKludges.C \
               ../../common/src/timing-linux.C \
					../../common/src/addrtranslate-static.C \
               ../../common/src/parseauxv.C

MYFLAGS += -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS

CXXFLAGS        += $(MYFLAGS)
CFLAGS          += $(MYFLAGS)
TFLAGS          += $(MYFLAGS)

# Tradeoff: adding -g to TFLAGS aids gdb but uses gobs of disk space
#TFLAGS                += -g

SYSLIBS                = -ldl 

LIBS    += -lgcc

AS             = /usr/bin/as


# Add preprocessor and special link information for G++
G_PTHREAD      = -pthread
G_PTHREAD_LD   = -pthread

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
