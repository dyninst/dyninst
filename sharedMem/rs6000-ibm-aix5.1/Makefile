#
# $Id: Makefile,v 1.1 2004/05/11 19:02:25 bernat Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

#DEBUG_FLAG = -g

# We change the permissions on the installed library to 0640
UNCOMMON_INSTALL = true
# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

VPATH   = ../src
#:../../dyninstAPI_RT/src

SRCS	= ../src/RTshared-unix.c 

LD              = $(LINKER)
LDFLAGS		+= -bnoentry -bnoobjreorder -bexpall -lc 

LDFLAGS +=  `$(GCC) -print-libgcc-file-name`

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

install: $(TARGET3_SO)
	-$(CP) $(TARGET3_SO) $(DEST)
	-chmod 640 $(DEST)/$(TARGET3_SO)
