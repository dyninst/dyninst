#
# $Id: Makefile,v 1.8 2001/02/01 01:08:45 schendel Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_SHM_SAMPLING = true

#If libhrtime is not installed on your computer and you have no interest
#  in using it, then comment out the following line (with a #), remove
#  the DEPENDS file in this directory, and recompile with make.
#If you have libhrtime, but it can't be found, the libhrtime path can 
#  be adjusted in ../../make.config.  You'll need to remake DEPENDS if
#  this path is adjusted.
USES_LIBHRTIME = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:
LD		= gcc
LDFLAGS         += -static -nostdlib -Xlinker -r
CFLAGS		+= -O2 

# The GNU headers for glibc conflict with RTposix.h
# CFLAGS		+= -D_SVID_SOURCE

# if we want to produce a shared object...
ifdef USES_LIBDYNINSTRT_SO
LD              = gcc
CFLAGS          = -Di386_unknown_linux2_0 -g
# CFLAGS		+= -D_SVID_SOURCE
CFLAGS		+= -fPIC
CFLAGS          += -I. -I../..
ifdef INCLUDE_CP_PROFILING
CFLAGS          += -I$(PVM_INCDIR)
endif
LDFLAGS         = -shared -ldl
else
TARGET2 = libdyninstRT.a
endif

VPATH        = ../src:../../dyninstAPI_RT/src

SRCS	     += ../src/RTfirst.c \
                ../../dyninstAPI_RT/src/RTheap.c \
		../../dyninstAPI_RT/src/RTheap-linux.c \
		../../dyninstAPI_RT/src/RTsig.c \
		../../dyninstAPI_RT/src/RTposix.c \
		../../dyninstAPI_RT/src/RTlinux.c \
		../src/RTetc-posix.c \
		../src/RTetc-linux.c \
		../src/RThwtimer-linux.c \
		../src/RTrexec.c

ifdef DETACH_ON_THE_FLY
CFLAGS += -DDETACH_ON_THE_FLY
SRCS 	+= ../../dyninstAPI_RT/src/RTsigill-x86.S
endif

ifdef USES_LIBHRTIME
CFLAGS  += -I$(HRTIME_INCDIR) -DHRTIME
LIBS	+= -L$(HRTIME_LIBDIR) $(HRTIME_LIB)
endif

#CFLAGS += -DSHM_SAMPLING_DEBUG
#CFLAGS += -DDEBUG_PRINT_RT     # enable debug/trace messages from library

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

