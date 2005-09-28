#
# $Id: Makefile,v 1.5 2005/09/28 17:02:36 bernat Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

BUILD_CODE_BLOCKS = true
#INCLUDE_CP_PROFILING = true

# Build using shared memory to communicate sample data instead of pipes.
# Much more efficient, and the standard.
USES_SHM_SAMPLING = true
#NO_OPT_FLAG = true
DEBUG_FLAG = -g

# We change the permissions on the installed library to 0640
OVERRIDE_INSTALL = true
# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

VPATH   = ../src
#:../../dyninstAPI_RT/src

SRCS	=  ../src/RTetc-aix.c \
                ../src/RTetc-posix.c \
                ../src/RTrexec.c \
                ../src/RTinst.c \
                ../src/RTmpi.c \
                ../src/RTthread.c \
                ../src/RTthread-timer.c \
                ../src/thread-compat.c

ifdef USES_NATIVE_CC

LD              = $(LINKER)
LDFLAGS		+= -bnoentry -bnoobjreorder -bexpall -lc -bM:SRE -L$(DEST)

else

LD		= $(CC) -shared

endif


# Initialization method
#LDFLAGS		+= -binitfini:libparadynRT_init

LDFLAGS += -L$(DEST) -ldyninstAPI_RT

ifdef USES_LIB_TEXT_HEAP
LDFLAGS += -lDyninstText
endif

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

#
# Don't optimize this module - jkh 6/21/95
#
RTrexec.o:
	$(CC) -c $(UNIFIED_DEF) -g ../src/RTrexec.c 

install: install_generic
	-chmod 640 $(DEST)/$(TARGET)
