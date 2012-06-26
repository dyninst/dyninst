#
# $Id: Makefile,v 1.1 2008/06/30 19:40:57 legendre Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

TARGET = libpcontrol.so

LDFLAGS     += $(LIBDIR) -lcommon

SRCS = ../src/linux.C \
       ../src/unix.C \
       ../src/notify_pipe.C \
       ../../common/src/dthread-unix.C

CXXFLAGS += $(MYFLAGS)
CFLAGS += $(MYFLAGS)

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

# Now make any necessary architecture specific changes to variables:

VPATH += ../../common/src

