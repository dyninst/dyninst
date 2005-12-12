#
# $Id: Makefile,v 1.2 2005/12/12 16:37:44 gquinn Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_SHM_SAMPLING = true

#If libhrtime is not installed on your computer and you have no interest
#  in using it, then comment out the following line (with a #), remove
#  the DEPENDS file in this directory, and recompile with make.
#If you have libhrtime, but it can't be found, the libhrtime path can 
#  be adjusted in ../../make.config.  You'll need to remake DEPENDS if
#  this path is adjusted.
#USES_LIBHRTIME = true
#USES_PAPI = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:
LD		= $(GCC)
LDFLAGS         += -shared -ldl 

# The GNU headers for glibc conflict with RTposix.h
# CFLAGS		+= -D_SVID_SOURCE

# if we want to produce a shared object...
CFLAGS          += -Di386_unknown_linux2_0 -g -O2
# CFLAGS		+= -D_SVID_SOURCE
CFLAGS		+= -fPIC
CFLAGS          += -I. -I../..

VPATH        = ../src
#:../../dyninstAPI_RT/src

SRCS	     += ../src/RTetc-linux.c \
		../src/RTetc-posix.c \
		../src/RThwtimer-linux.c \
		../src/RTrexec.c \
		../src/RTinst.c \
		../src/RTmpi.c \
		../src/RTthread.c \
		../src/RTthread-timer.c \
		../src/thread-compat.c

ifdef USES_LIBPERFCTR
CFLAGS  += -I$(PERFCTR_INCDIR) -DPERFCTR
LIBS	+= -L$(PERFCTR_LIBDIR) $(PERFCTR_LIB)
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

