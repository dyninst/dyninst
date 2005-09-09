#
# $Id: Makefile,v 1.18 2005/09/09 18:05:07 legendre Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

# none

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

USES_LIBELF = true

include ../../make.config 

# Now make any necessary architecture specific changes to variables:
LD		= $(GCC)
LDFLAGS         += -shared -nostdlib

# Automatic init routine
LDFLAGS		+= -Wl,-init -Wl,libdyninstAPI_RT_init

CFLAGS		+= -O2 

# if we want to produce a shared object...
CC              = $(GCC)
LD              = $(GCC)
CFLAGS          += -I. -I../.. -I$(LIBELF_INC) -I/usr/include/libelf -fpic 

LIBS 		+= -ldl -L$(LIBELF_LIB)
ifdef LIBELFSTATIC
LIBS    += -Wl,-static -lelf -Wl,-call_shared
else
LIBS    += -lelf
endif

AS = gcc
ASFLAGS         += -c

SRCS		+= ../src/RTposix.c \
		   ../src/RTlinux.c \
		   ../src/RTheap.c \
         ../src/RTheap-linux.c \
		   ../src/RTmutatedBinary.c \
			../src/RTmutatedBinary_ELF.c \
			../src/RTthread.c \
			../src/RTthread-index.c \
			../src/RTthread-x86.c

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

