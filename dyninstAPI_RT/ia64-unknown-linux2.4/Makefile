#
# $Id: Makefile,v 1.5 2004/03/05 16:51:15 bernat Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

DEBUG_FLAG = -g

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Set platform-specific variables for this build.
LD				= $(GCC)
LDFLAGS			+= -static -nostdlib -Xlinker -r

CFLAGS			+= -O2 

CC				= $(GCC)
LD				= $(GCC)
CFLAGS			+= -I. -I../.. -fpic
LDFLAGS			= -shared -ldl -Wl,-init,_start

ASFLAGS			= 

SRCS		+= \
			../src/RTposix.c \
			../src/RTlinux.c \
			../src/RTsig.c \
			../src/RTheap.c \
			../src/RTheap-linux.c

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

