#
# $Id: Makefile,v 1.7 2004/04/28 20:21:00 rchen Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_UNWIND_LIB = true

# Turns off optimization, and enables debugging.
# NO_OPT_FLAG = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.
include ../../make.config 

# Set platform-specific variables for this build.
LD			= $(GCC)
LDFLAGS			+= -static -nostdlib -Xlinker -r

CC			= $(GCC)
LD			= $(GCC)
CFLAGS			+= -I. -I../.. -fpic
LDFLAGS			= -shared -ldl -Wl,-init,_start \
				  -L$(LIBUNWINDDIR)/lib \
				  -Wl,--whole-archive -Wl,-Bstatic -lunwind -Wl,-Bdynamic -Wl,--no-whole-archive

ASFLAGS			= 

SRCS			+= \
			../src/RTposix.c \
			../src/RTlinux.c \
			../src/RTsig.c \
			../src/RTheap.c \
			../src/RTheap-linux.c

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

