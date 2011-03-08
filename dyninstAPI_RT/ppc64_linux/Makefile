#
# $Id: Makefile,v 1.4 2008/02/20 08:31:08 jaw Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

# none

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

ifndef STATIC_TARGET
    STATIC_TARGET = libdyninstAPI_RT.a
endif

# Now make any necessary architecture specific changes to variables:
LD		= $(GCC)
LDFLAGS         += -shared

# Automatic init routine
#LDFLAGS		+= -Wl,-init -Wl,libdyninstAPI_RT_init

CFLAGS		+= -O2 

# if we want to produce a shared object...
CC              = $(GCC)
LD              = $(GCC)
STATIC_CFLAGS  := $(CFLAGS) -DDYNINST_RT_STATIC_LIB -I. -I../..
CFLAGS          += -I. -I../.. -fpic

LIBS 		+= -ldl

AS = gcc
STATIC_ASFLAGS  := $(ASFLAGS) -c -DDYNINST_RT_STATIC_LIB
ASFLAGS         += -c

SRCS		+= ../src/RTposix.c \
		   ../src/RTlinux.c \
		   ../src/RTheap.c \
		   ../src/RTheap-linux.c \
		   ../src/RTthread.c \
		   ../src/RTthread-powerpc.c \
		   ../src/RTthread-powerpc-asm.S \
		   ../src/RTspace.S
#		   ../src/RTmutatedBinary.c \
#		   ../src/RTmutatedBinary_ELF.c \

#CFLAGS += -DDEBUG_PRINT_RT    # enable debug/trace messages from library

ifdef DETACH_ON_THE_FLY
CFLAGS += -DDETACH_ON_THE_FLY
SRCS += ../src/RTsigill-powerpc.S
endif

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

RTthread-powerpc-asm.o:
	$(CC) -c $(CFLAGS) ../src/RTthread-powerpc-asm.S

RTspace.o:
	$(CC) -c $(CFLAGS) ../src/RTspace.S

