#
# $Id: Makefile,v 1.2 2003/04/11 16:18:33 bernat Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

BUILD_CODE_BLOCKS = true
#INCLUDE_CP_PROFILING = true

# Build using shared memory to communicate sample data instead of pipes.
# Much more efficient, and the standard.
USES_SHM_SAMPLING = true
#NO_OPT_FLAG = true

# We change the permissions on the installed library to 0640
UNCOMMON_INSTALL = true
# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

VPATH   = ../src
#:../../dyninstAPI_RT/src

SRCS	= ../src/RTfirst.c \
	   ../src/RTetc-posix.c \
	   ../src/RTetc-aix.c \
	   ../src/RTrexec.c

LD              = $(LINKER)
LDFLAGS		+= -bnoentry -bnoobjreorder -bexpall -lc 

# Initialization method
LDFLAGS		+= -binitfini:libparadynRT_init

LDFLAGS +=  `$(GCC) -print-libgcc-file-name`

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

#
# Define the objects by hand since we need the exact ordering and 
#   the auto rules don't provide that.
OBJS	=  RTfirst.o \
	   RTetc-aix.o \
	   RTetc-posix.o \
	   RTinst.o \
	   RTrexec.o \
	   RTend.o

#
# Don't optimize this module - jkh 6/21/95
#
RTrexec.o:
	$(CC) -c $(ARCH_DEF) -g ../src/RTrexec.c 

install: $(TARGET3_SO)
	-$(CP) $(TARGET3_SO) $(DEST)
	-chmod 640 $(DEST)/$(TARGET3_SO)
