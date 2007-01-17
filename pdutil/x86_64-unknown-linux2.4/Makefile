#
# $Id: Makefile,v 1.2 2007/01/17 22:47:52 giri Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

NO_IMPLICIT_TEMPLATES = true
DO_INLINE_ALL		= true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Now make any necessary architecture specific changes to variables:

CFLAGS   += -I/usr/include/libelf
CXXFLAGS += $(USEFULWARNINGS) -I/usr/include/libelf

# no support for elf64_XXX() functions
CXXFLAGS += -DUSES_ELF32_ONLY
#CXXFLAGS += -DAGGREGATE_DEBUG
#CXXFLAGS += -DHIST_DEBUG
CFLAGS   += -DUSES_ELF32_ONLY

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl


