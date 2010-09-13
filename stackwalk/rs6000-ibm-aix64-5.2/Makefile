#
# Define any symbols needed to invoke configuration changes in make.config

# AIX uses .a for library names...
OVERRIDE_INSTALL = true
TARGET	= libstackwalk.a

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS =  ../src/aix-swk.C \
		  ../src/ppc-swk.C \
		  ../src/firstparty.C \
		  ../src/symtab-swk.C

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# On AIX all symbols must be resolved at link time, even for a library.
LDFLAGS += -Wl,-bbigtoc,-bexpall -lsymtabAPI
# Override default of gcc -- need g++ on AIX
LD = $(GXX)

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

#
# Install rule: install withOUT world-readable permissions
# 
install: install_generic
	-chmod 750 $(DEST)/$(TARGET)
