#
# $Id: Makefile,v 1.3 2003/05/10 23:52:57 schendel Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_SHM_SAMPLING = true

# We set the permissions of the runtime library as 640
UNCOMMON_INSTALL = true
DEBUG_FLAG = -g

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

# Build and link the library text heap
ifdef USES_LIB_TEXT_HEAP
USES_LIB_TEXT_HEAP_INTERNAL = true # Second level define
# Change this path to your library install location
LIB_TEXT_HEAP_OBJ = libDyninstText.a
CFLAGS	+= -DUSES_LIB_TEXT_HEAP
LDFLAGS += -L$(DEST) -lDyninstText
endif


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

libDyninstText.a:
	$(CC) -c -o libSpace.o ../src/libSpace.s
	ld -o space.o -bexpall -bnoentry -bM:SRE libSpace.o
	rm -f libSpace.o
	rm -f libDyninstText.a
	ar crv libDyninstText.a space.o
	-$(CP) libDyninstText.a $(DEST)
	-chmod 755 $(DEST)/libDyninstText.a
	rm -f space.o

install: $(TARGET3_SO)
	-$(CP) $(TARGET3_SO) $(DEST)
	-chmod 640 $(DEST)/$(TARGET3_SO)
