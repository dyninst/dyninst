# Define any symbols needed to invoke configuration changes in make.config

USES_SHM_SAMPLING = true

# USES_LIBHRTIME = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:
LD			= $(GCC)
LDFLAGS		+= -static -nostdlib -Xlinker -r
CFLAGS		+= -O2 

# if we want to produce a shared object...
LD			= $(GCC)
CFLAGS		= -Dia64_unknown_linux2_4 -g -O2
CFLAGS		+= -fPIC
CFLAGS		+= -I. -I../..
LDFLAGS		= -shared -ldl

VPATH		= ../src:../../dyninstAPI_RT/src

SRCS		+=	../src/RTfirst.c \
				../../dyninstAPI_RT/src/RTheap.c \
				../../dyninstAPI_RT/src/RTheap-linux.c \
				../../dyninstAPI_RT/src/RTsig.c \
				../../dyninstAPI_RT/src/RTposix.c \
				../../dyninstAPI_RT/src/RTlinux.c \
				../src/RTetc-posix.c \
				../src/RTetc-linux.c \
				../src/RThwtimer-linux.c \
				../src/RTrexec.c

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
