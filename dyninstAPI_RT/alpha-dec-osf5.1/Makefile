#
# $Id: Makefile,v 1.2 2004/03/05 16:51:13 bernat Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

#USES_SHM_SAMPLING = true

include               ../../make.config

CFLAGS	+= 
#CFLAGS += -DUSE_PROF

SRCS         += ../src/RTposix.c \
		../src/RTosf.c \
		../src/RTheap.c \
		../src/RTheap-osf.c \
		../src/RTheap-svr4.c \
		../src/RTsave_alpha.S \
		../src/RTtext-alpha.S


LD		= $(GCC)
LDFLAGS		+= -static -nostdlib -Xlinker -r
ASFLAGS		+= -I../..

# if we want to produce a shared object...
CC              = $(GCC)
AS		= as
LD              = $(GCC)
CFLAGS          += -I. -I../.. -Dexternal_templates -fPIC
LDFLAGS         = -msym -shared -taso -lc
LDFLAGS         = -shared 

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

