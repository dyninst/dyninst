#
# $Id: Makefile,v 1.1 2008/06/30 19:40:57 legendre Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

NO_IMPLICIT_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

STATIC_COMPS = true

include ../../make.config 

TARGET = libstackwalk.a

LDFLAGS     += $(LIBDIR) -lcommon

SRCS = 	../src/linux-swk.C \
	../src/linuxbsd-x86-swk.C \
	../src/x86-swk.C \
	../src/symtab-swk.C \
	../src/dbginfo-stepper.C \
	../src/x86-wanderer.C \
	../src/analysis_stepper.C \
	../src/linuxbsd-swk.C \
	../src/linux-x86-swk.C \
	../../common/src/Elf_X.C

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

# Now make any necessary architecture specific changes to variables:


CFLAGS  += -fno-omit-frame-pointer -Dcap_sw_catchfaults $(MYFLAGS)
CXXFLAGS  += -fno-omit-frame-pointer -Dcap_sw_catchfaults $(MYFLAGS)
AS		= as
VPATH += ../../common/src
