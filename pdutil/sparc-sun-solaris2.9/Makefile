#
# $Id: Makefile,v 1.1 2003/09/30 14:20:46 jodom Exp $
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
SRCS     += ../../common/src/solarisKludges.C \
            ../../common/src/timing-solaris.C

OBJS     += solarisKludges.o \
            timing-solaris.o

solarisKludges.o: ../../common/src/solarisKludges.C
	$(CXX) -c $(KFLAGS) ../../common/src/solarisKludges.C

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
