#
# $Id: Makefile,v 1.2 2001/06/04 19:26:06 bernat Exp $
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

# if we want to produce a shared object...

CFLAGS          = -g -I. -I../.. -Dsparc_sun_solaris2_4
ifdef USES_NATIVE_CC
CC		= $(NATIVE_CC)
LD		= $(NATIVE_CC)
CFLAGS          += -Kpic
LDFLAGS         = -dy -G -z text
else
CC              = gcc
LD              = gcc
CFLAGS          += -fpic
LDFLAGS		= -shared
endif

#CFLAGS += -DDEBUG_PRINT_RT      # enable debug/trace messages from library

SRCS		+= ../src/RTposix.c \
		   ../src/RTsolaris.c \
		   ../src/RTheap.c \
		   ../src/RTheap-solaris.c \
		   ../src/RTheap-svr4.c

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

