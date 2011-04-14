#
# $Id: Makefile,v 1.2 2008/02/20 08:31:07 jaw Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

# none

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

UNIFIED_DEF += -D__RTLIB__

# Now make any necessary architecture specific changes to variables:
LD		= $(GCC)
LDFLAGS         += -nostdlib

# Automatic init routine
LDFLAGS		+= -Wl,-init -Wl,libdyninstAPI_RT_init

CFLAGS		+= -O2 

# if we want to produce a shared object...
GCC             = ccpentium -DCPU=SIMLINUX
CC              = $(GCC)
LD              = $(GCC)
CFLAGS          += -I. -I../..

# LIBS 		+= -ldl

AS              = $(GCC)
ASFLAGS         += -c

SRCS		+= ../src/RTposix.c \
		   ../src/RTheap.c \
		   ../src/RTthread.c \
		   ../src/RTthread-index.c \
		   ../src/RTvxworks.c \
		   ../src/RTthread-x86.c

#		   ../src/RTthread-powerpc-asm.S \
#CFLAGS += -DDEBUG_PRINT_RT    # enable debug/trace messages from library

ifdef DETACH_ON_THE_FLY
CFLAGS += -DDETACH_ON_THE_FLY
SRCS += ../src/RTsigill-x86.S
endif

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

RTspace.o:
	$(CC) -c $(CFLAGS) ../src/RTspace.S

