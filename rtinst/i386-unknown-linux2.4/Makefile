#
# $Id: Makefile,v 1.1 2000/03/31 01:59:28 wylie Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_SHM_SAMPLING = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:
LD		= gcc
LDFLAGS         += -static -nostdlib -Xlinker -r

DYNOBJS		= DYNINSTstartCode.o \
		DYNINSTendCode.o \
		RTpvmPiggy.o \
		RTcriticalPath.o

CFLAGS		+= -O2 

# The GNU headers for glibc conflict with RTposix.h
# CFLAGS		+= -D_SVID_SOURCE

# if we want to produce a shared object...
ifdef USES_LIBDYNINSTRT_SO
LD              = gcc
CFLAGS          = -Di386_unknown_linux2_0 -g -Dexternal_templates
# CFLAGS		+= -D_SVID_SOURCE
CFLAGS		+= -fPIC
CFLAGS          += -I. -I../..
ifdef INCLUDE_CP_PROFILING
CFLAGS          += -I$(PVM_INCDIR)
endif
LDFLAGS         = -shared
else
TARGET2 = libdyninstRT.a
endif

SRCS	     += ../src/RTfirst.c \
		../src/RTlinux.c \
		../src/RTposix.c \
		../src/RTrexec.c \
                ../src/RTheap.c \
		../src/RTheap-linux.c

#CFLAGS += -DSHM_SAMPLING_DEBUG

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

ifdef INCLUDE_CP_PROFILING
$(TARGET2): $(OBJS) $(CP_OBJS)
	@$(RM) $(TARGET2)
	$(LD) $(LDFLAGS) -o $(TARGET2) $(OBJS) $(CP_OBJS) $(LIBS)
endif

