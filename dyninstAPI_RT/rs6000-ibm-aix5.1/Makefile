#
# $Id: Makefile,v 1.1 2003/03/31 17:45:56 buck Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_SHM_SAMPLING = true

# We set the permissions of the runtime library as 640
UNCOMMON_INSTALL = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include               ../../make.config

CFLAGS	+= 
#CFLAGS	+= -DDEBUG_PRINT_RT     # enable debug/trace messages from library
#CFLAGS += -DUSE_PROF
ASFLAGS += -I../..

SRCS         += ../src/RTposix.c \
		../src/RTaix.c \
                ../src/RTheap.c \
	        ../src/RTheap-aix.c \
		../src/RTmutatedBinary.c \
		../src/RTmutatedBinary_XCOFF.c

LD		= $(LINKER)

LDFLAGS		+= -bnoobjreorder -bexpall -bnoentry  -lc
# Initialization method
LDFLAGS		+= -binitfini:libdyninstAPI_RT_init

# Do we want to use cc (xlc) or gcc? If cc, then life is a little
# more complicated
# Check at the bottom of the file as well for one more ifdef
ifdef USES_NATIVE_CC
CC = $(NATIVE_CC)
else
# Include libgcc in the ldflags -- -L<path to libgcc> -lgcc
LDFLAGS += `$(GCC) -print-libgcc-file-name`
endif

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

install: $(TARGET3_SO)
	-$(CP) $(TARGET3_SO) $(DEST)
	-chmod 640 $(DEST)/$(TARGET3_SO)
