#
# $Id: Makefile,v 1.22 2008/02/20 08:31:07 jaw Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

# none

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Right now, only enable for platforms that need it
ifndef STATIC_TARGET
    STATIC_TARGET = libdyninstAPI_RT.a
endif

# Now make any necessary architecture specific changes to variables:
LD		= $(GCC)
LDFLAGS         += -shared
LDFLAGS		+= -m64
CFLAGS		+= -Wall -m64 -DMUTATEE64

# if we want to produce a shared object...
CC              = $(GCC)
LD              = $(GCC)
STATIC_CFLAGS   := $(CFLAGS) -DDYNINST_RT_STATIC_LIB -I. -I../..
CFLAGS          += -I. -I../.. -fpic

LIBS 		+=

AS = gcc
STATIC_ASFLAGS := $(ASFLAGS) -c -DDYNINST_RT_STATIC_LIB
ASFLAGS         += -c

SRCS		+= ../src/RTposix.c \
		   ../src/RTfreebsd.c \
		   ../src/RTheap.c \
		   ../src/RTheap-freebsd.c \
		   ../src/RTthread.c \
		   ../src/RTthread-x86-64.c \
		   ../src/RTspace.S \
		   ../src/RTtlsgetaddr-x86_64.S \
		   ../src/RTstatic_ctors_dtors-x86.c

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

