#
# $Id: Makefile,v 1.1 2003/09/30 14:20:45 jodom Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

# none

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

NATIVE_CC = cc
ifneq (,$(findstring WorkShop,$(shell $(NATIVE_CC) -V 2>&1)))
USES_NATIVE_CC = true
endif
ifneq (,$(findstring Sun C,$(shell $(NATIVE_CC) -V 2>&1)))
USES_NATIVE_CC = true
endif

# if we want to produce a shared object...

CFLAGS          = -g -I. -I../.. -Dsparc_sun_solaris2_4
ifdef USES_NATIVE_CC
CC		= $(NATIVE_CC)
LD		= $(NATIVE_CC)
CFLAGS          += -Kpic -D_NATIVESO_
LDFLAGS         = -ldl -lelf -dy -G -z text -z initarray=libdyninstAPI_RT_init
else
CC              = $(GCC)
LD              = $(GCC)
CFLAGS          += -fpic
LDFLAGS		= -ldl -lelf -shared -nostdlib -lgcc -z initarray=libdyninstAPI_RT_init
#ccw 19 nov 2001
endif

#CFLAGS += -DDEBUG_PRINT_RT      # enable debug/trace messages from library

SRCS		+= ../src/RTposix.c \
		   ../src/RTsolaris.c \
		   ../src/RTheap.c \
		   ../src/RTheap-solaris.c \
		   ../src/RTheap-svr4.c \
			../src/RTmutatedBinary.c \
			../src/RTmutatedBinary_ELF.c

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

