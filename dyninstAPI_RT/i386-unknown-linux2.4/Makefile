#
# $Id: Makefile,v 1.14 2004/05/12 22:37:47 legendre Exp $
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
CFLAGS          += -I. -I../.. -I$(LIBELFDIR)/include -I/usr/include/libelf -fpic 

LIBS 		+= -ldl -L$(LIBELFDIR)/bin
ifdef LIBELFSTATIC
LIBS    += -Wl,-static -lelf -Wl,-call_shared
else
LIBS    += -lelf
endif

ASFLAGS         += 

SRCS		+= ../src/RTposix.c \
		   ../src/RTlinux.c \
		   ../src/RTsig.c \
		   ../src/RTheap.c \
         ../src/RTheap-linux.c \
		   ../src/RTmutatedBinary.c \
			../src/RTmutatedBinary_ELF.c

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

