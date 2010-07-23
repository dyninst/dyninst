#
# $Id: Makefile,v 1.8 2008/05/27 20:44:58 giri Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

DEBUG_FLAG = -g

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

CXXFLAGS        += $(MYFLAGS)
CFLAGS          += $(MYFLAGS)
TFLAGS          += $(MYFLAGS)
AS		= /usr/bin/as

# Add preprocessor and special link information for G++
G_PTHREAD       = -pthread

TARGET = libdataflowAPI.so

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
