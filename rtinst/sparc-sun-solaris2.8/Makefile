#
# $Id: Makefile,v 1.4 2002/07/31 22:07:00 bernat Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_SHM_SAMPLING = true
#BUILD_CODE_BLOCKS = true
#INCLUDE_CP_PROFILING = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture-specific changes to variables:

NATIVE_CC = cc
ifneq (,$(findstring WorkShop,$(shell $(NATIVE_CC) -V 2>&1)))
USES_NATIVE_CC = true
endif

CFLAGS		= -g -I. -I../.. -Dsparc_sun_solaris2_4
ifdef INCLUDE_CP_PROFILING
CFLAGS		+= -I$(PVM_INCDIR)
endif
ifdef USES_NATIVE_CC
CC		= $(NATIVE_CC)
LD		= $(NATIVE_CC)
CFLAGS		+= -Kpic -O
LDFLAGS		= -dy -G -z text
else
CC		= $(GCC)
LD		= $(GCC)
CFLAGS		+= -fpic -O2
LDFLAGS		= -shared
endif

VPATH        = ../src
#:../../dyninstAPI_RT/src # ccw 18 apr 2002 : SPLIT

SRCS	     += ../src/RTfirst.c \
		../src/RTetc-posix.c \
		../src/RTetc-solaris.c \
		../src/RTrexec.c

#		../../dyninstAPI_RT/src/RTheap.c \ # ccw 18 apr 2002 : SPLIT
#		../../dyninstAPI_RT/src/RTheap-svr4.c \
#		../../dyninstAPI_RT/src/RTheap-solaris.c \
#		../../dyninstAPI_RT/src/RTposix.c \
#		../../dyninstAPI_RT/src/RTsolaris.c \

#CFLAGS += -DDEBUG_PRINT_RT     # enable debug/trace messages from library
#CFLAGS += -DSHM_SAMPLING_DEBUG # shared-memory sampling debug/trace

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
