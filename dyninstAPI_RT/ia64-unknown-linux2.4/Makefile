#
# $Id: Makefile,v 1.1 2002/02/11 22:01:28 tlmiller Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

# none

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:
LD		= gcc
LDFLAGS         += -static -nostdlib -Xlinker -r

CFLAGS		+= -O2 

# if we want to produce a shared object...
CC              = gcc
LD              = gcc
CFLAGS          = -I. -I../.. -Dia64_unknown_linux2_4 -g -fpic
LDFLAGS         = -shared -ldl

ASFLAGS         = 

SRCS		+= ../src/RTposix.c \
		   ../src/RTlinux.c \
		   ../src/RTsig.c \
		   ../src/RTheap.c \
                   ../src/RTheap-linux.c

#CFLAGS += -DDEBUG_PRINT_RT    # enable debug/trace messages from library

ifdef DETACH_ON_THE_FLY
CFLAGS += -DDETACH_ON_THE_FLY
SRCS += ../src/RTsigill-ia64.S
endif

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

