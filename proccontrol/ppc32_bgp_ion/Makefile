#
# $Id: Makefile,v 1.1 2008/06/30 19:40:57 legendre Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# STATIC_COMPS = true

# TARGET = libpcontrol.a

LDFLAGS     += $(LIBDIR) -lcommon

SRCS =  ../src/bluegene.C \
        ../src/notify_pipe.C \
        ../src/sysv.C \
        ../src/ppc_process.C \
        ../../common/src/dthread-unix.C

ifdef LIBELF_INC
MYFLAGS += -I$(LIBELF_INC)
endif
MYFLAGS    += -I/bgsys/drivers/ppcfloor/arch/include

CXXFLAGS += $(MYFLAGS)
CFLAGS += $(MYFLAGS)

ifdef LIBELF_LIB
LIBS += -L$(LIBELF_LIB)
endif
LIBS += -lelf

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

# Now make any necessary architecture specific changes to variables:

VPATH += ../../common/src
