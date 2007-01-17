#
# $Id: Makefile,v 1.2 2007/01/17 22:47:51 giri Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

NO_IMPLICIT_TEMPLATES = true
DO_INLINE_ALL		= true

# Turn off ALL optimization to make debuggering easier.  Remember
#  to remove (and re-compile w/optimization) before committing....
#NO_OPT_FLAG=true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

#CXXFLAGS        +=  -DHIST_DEBUG
#CXXFLAGS        +=  -DAGGREGATE_DEBUG

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Now make any necessary architecture specific changes to variables:

CXXFLAGS += $(USEFULWARNINGS)

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
