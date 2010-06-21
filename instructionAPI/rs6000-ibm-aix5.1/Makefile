#
# $Id: Makefile,v 1.3 2008/09/04 21:06:52 bill Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

# DEBUG_FLAG = -g
AUTO_TEMPLATES= true
#NO_IMPLICIT_TEMPLATES = true
USES_BOOST = true

# For debugging.
#NO_OPT_FLAG = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS = ../src/InstructionDecoder-x86.C

CXXFLAGS        += $(MYFLAGS)
CFLAGS          += $(MYFLAGS)

LIBS    += -lgcc

AS		= /usr/bin/as

TARGET = libinstructionAPI.a
# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
