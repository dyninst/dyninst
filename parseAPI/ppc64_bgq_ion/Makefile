#
# $Id: Makefile,v 1.3 2008/09/15 18:37:49 jaw Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

#DEBUG_FLAG = -g
AUTO_TEMPLATES= true
#NO_IMPLICIT_TEMPLATES = true
USES_BOOST = true

# For debugging.
#NO_OPT_FLAG = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

#STATIC_COMPS = true

TO_CORE = ../..

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:
MYFLAGS         += -g

CXXFLAGS        += $(MYFLAGS)
CFLAGS          += $(MYFLAGS)

SRCS =  ../src/InstructionSource-power.C \
        ../src/IA_IAPI.C \
        ../src/IA_power.C

LIBS    += -lgcc

AS		= /usr/bin/as

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
