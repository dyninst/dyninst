#
# $Id: Makefile,v 1.7 2005/09/13 00:23:15 legendre Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_SHM_SAMPLING = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:
LD				= $(GCC)
LDFLAGS			= -shared -ldl

CFLAGS          += -Dia64_unknown_linux2_4 -g -O2
CFLAGS			+= -fPIC
CFLAGS          += -I. -I../..

VPATH			= ../src

SRCS			+= ../src/RTetc-posix.c \
					../src/RTetc-linux.c \
					../src/RThwtimer-linux.c \
					../src/RTrexec.c \
					../src/RTinst.c \
					../src/RTmpi.c \
					../src/RTthread.c \
					../src/RTthread-timer.c

ifdef USES_LIBHRTIME
	CFLAGS		+= -I$(HRTIME_INCDIR) -DHRTIME
	LIBS		+= -L$(HRTIME_LIBDIR) $(HRTIME_LIB)
endif

# enable debug/trace messages from library
# CFLAGS			+= -DSHM_SAMPLING_DEBUG
# CFLAGS			+= -DDEBUG_PRINT_RT

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

