#
# $Id: Makefile,v 1.1 2008/06/30 19:40:57 legendre Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_LIBELF = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

TARGET = libpcontrol.so

LDFLAGS     += $(LIBDIR) -lcommon

SRCS = ../src/sysv.C \
       ../src/freebsd.C \
       ../src/unix.C \
       ../src/x86_process.C \
       ../../common/src/dthread-unix.C \
       ../../common/src/Elf_X.C

MYLAGS += -I$(LIBELF_DIR)
endif

CXXFLAGS += $(MYFLAGS)
CFLAGS += $(MYFLAGS)

ifdef LIBELF_INC
CXXFLAGS += -I$(LIBELF_INC) 
CFLAGS += -I$(LIBELF_INC)
endif
ifdef LIBELF_LIB
LIBS += -L$(LIBELF_LIB)
endif
LIBS += -lelf -lthread_db -pthread

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

# Now make any necessary architecture specific changes to variables:
VPATH += ../../common/src 
