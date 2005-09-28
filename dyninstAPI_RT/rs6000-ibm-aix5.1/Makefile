#
# $Id: Makefile,v 1.9 2005/09/28 17:02:30 bernat Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_SHM_SAMPLING = true

# We set the permissions of the runtime library as 640
OVERRIDE_INSTALL = true
DEBUG_FLAG = -g

# AIX: libraries end in .a
TARGET = libdyninstAPI_RT.a
TARGET_SYMLINK = libdyninstAPI_RT.so.1

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include               ../../make.config

CFLAGS	+= -Wa,-mppc
#CFLAGS	+= -DDEBUG_PRINT_RT     # enable debug/trace messages from library
#CFLAGS += -DUSE_PROF
ASFLAGS += -I../..

SRCS  += ../src/RTposix.c \
	../src/RTaix.c \
	../src/RTheap.c \
	../src/RTheap-aix.c \
	../src/RTmutatedBinary.c \
	../src/RTmutatedBinary_XCOFF.c \
	../src/RTthread-power-asm.s \
	../src/RTthread-index.c \
	../src/RTthread.c

# Build and link the library text heap
ifdef USES_LIB_TEXT_HEAP
EXTRA_LIBS = libDyninstText.a
CFLAGS	+= -DUSES_LIB_TEXT_HEAP
LDFLAGS += -L. -lDyninstText
endif

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

# If we're not using the native compiler, use gcc as a linker
ifdef USES_NATIVE_CC
LD		= $(LINKER) 
LDFLAGS		+= -bnoobjreorder -bexpall -bnoentry  -lc
# Initialization method
LDFLAGS		+= -binitfini:libdyninstAPI_RT_init

libDyninstText.a: space.o
#	rm -f libSpace.o 
#	rm -f libDyninstText.a 
	ld -o space_lib.o -bexpall -bnoentry -bM:SRE space.o 
	ar crv libDyninstText.a space_lib.o 
	rm -f space_lib.o
#	rm -f space.o


else
# GCC
LD		= $(CC) -shared
# Initialization method
LDFLAGS		+= -Wl,-binitfini:libdyninstAPI_RT_init

libDyninstText.a: space.o
	gcc -shared -o libDyninstText.a space.o

endif


libDyninstAPI_RT.so.1: libDyninstText.a
# ... and uses default AIX link line

RTthread-power-asm.o:
	$(CC) -c $(CFLAGS) ../src/RTthread-power-asm.s

space.o: 
	$(CC) -c -o space.o ../src/libSpace.s

install: install_generic libDyninstText.a
	-chmod 640 $(DEST)/$(TARGET)
	-$(CP) libDyninstText.a $(DEST)
	-chmod 755 $(DEST)/libDyninstText.a
	-ln -s $(TARGET) $(DEST)/$(TARGET_SYMLINK)
