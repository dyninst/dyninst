#
# $Id: Makefile,v 1.1 2003/05/25 01:06:37 rchen Exp $
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


CFLAGS	     +=  -Dalpha_dec_osf4_0 -I../.. -I/usr/include/ -I../h/
LD		= $(GCC)
LDFLAGS		+= -static -nostdlib -Xlinker -r
ASFLAGS		+= -I../..

# if we want to produce a shared object...
CC              = $(GCC)
AS		= as
LD              = $(GCC)
CFLAGS          = -I. -I../.. -Dalpha_dec_osf4_0 -g -Dexternal_templates -fPIC
LDFLAGS         = -msym -shared -taso -lc
LDFLAGS         = -shared 

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

