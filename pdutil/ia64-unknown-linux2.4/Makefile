#
# $Id: Makefile,v 1.2 2004/07/23 20:39:13 tlmiller Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

NO_IMPLICIT_TEMPLATES	= true
DO_INLINE_ALL			= true
USES_LIBELF				= true

NO_OPT_FLAG = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Now make any necessary architecture specific changes to variables:

CFLAGS   += -g -I$(LIBELF_INC)
CXXFLAGS += -g $(USEFULWARNINGS) -I$(LIBELF_INC)
SRCS     += ../../common/src/linuxKludges.C \
            ../../common/src/timing-linux.C

OBJS     += linuxKludges.o \
            timing-linux.o

linuxKludges.o: ../../common/src/linuxKludges.C
	$(CXX) -c $(KFLAGS) ../../common/src/linuxKludges.C

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
