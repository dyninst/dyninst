#
# $Id: Makefile,v 1.4 2003/09/23 17:29:00 tlmiller Exp $
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
CFLAGS			= -I. -I../.. -Dia64_unknown_linux2_4 -g -fpic
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

