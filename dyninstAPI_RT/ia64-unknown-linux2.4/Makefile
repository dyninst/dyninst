#
# $Id: Makefile,v 1.15 2006/04/03 01:11:50 mirg Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_UNWIND_LIB = true

# Turns off optimization, and enables debugging.
# NO_OPT_FLAG = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.
include ../../make.config 

# Set platform-specific variables for this build.
CC			= $(GCC)
LD			= $(GCC)
CFLAGS		+= -I. -I../.. -fpic 
LDFLAGS		+=	-shared -ldl -L$(LIBUNWIND_LIB) -lunwind 

# Automatic init routine
LDFLAGS		+= -Wl,-init -Wl,libdyninstAPI_RT_init

ASFLAGS		= 

SRCS		+= \
				../src/RTposix.c \
				../src/RTlinux.c \
				../src/RTheap.c \
				../src/RTheap-linux.c \
				../src/RTthread.c \
				../src/RTthread-index.c \
				../src/RTthread-ia64.c

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
