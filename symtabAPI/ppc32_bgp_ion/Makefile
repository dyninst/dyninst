#
# $Id: Makefile,v 1.1 2007/08/01 18:47:41 ssuen Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

NO_IMPLICIT_TEMPLATES = true

STATIC_COMPS = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.
include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS =          ../src/Object-elf.C \
                ../src/Archive.C \
                ../src/Archive-elf.C \
                ../src/parseDwarf.C \
                ../src/parseStab.C \
                ../src/emitElf.C \
		../src/emitElfStatic.C \
		../src/emitElfStatic-stub.C \
		../src/LinkMap.C \
		../src/relocationEntry-stub.C

MYFLAGS                += -DUSES_DWARF_DEBUG

CXXFLAGS        += $(MYFLAGS) -I/bgsys/drivers/ppcfloor/arch/include
CFLAGS          += $(MYFLAGS)
TFLAGS          += $(MYFLAGS)

SYSLIBS                = -ldl 

AS             = /usr/bin/as

# no support for elf64_XXX() functions
# CXXFLAGS     += -DUSES_ELF32_ONLY
# CFLAGS       += -DUSES_ELF32_ONLY
# TFLAGS       += -DUSES_ELF32_ONLY

# Add preprocessor and special link information for G++

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

# Now make any necessary architecture specific changes to variables:
VPATH += ../../common/src
