#
# $Id: Makefile,v 1.8 2002/02/26 15:49:46 pcroth Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

NO_IMPLICIT_TEMPLATES = true
DO_INLINE_ALL           = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 


ifdef USES_PMAPI
CFLAGS			+=	-DUSES_PMAPI
CXXFLAGS		+=	-DUSES_PMAPI
endif

#CXXFLAGS        +=  -DHIST_DEBUG
#CXXFLAGS        +=  -DAGGREGATE_DEBUG

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Now make any necessary architecture specific changes to variables:

CXXFLAGS        += $(USEFULWARNINGS)
SRCS            += ../../common/src/aixv41Kludges.C \
                   ../../common/src/timing-aix.C
OBJS            += aixv41Kludges.o \
                   timing-aix.o

# you'll need the following for the SP2/AIX if your sscanf is also broken
# and unable to handle a numeric field width for the first field (eg. "%16s")
CXXFLAGS	+= -DBROKEN_SSCANF

ifdef CROSSCOMPILER
KFLAGS		+= -DCROSSCOMPILER
endif

aixv41Kludges.o: ../../common/src/aixv41Kludges.C
	$(CXX) -c $(KFLAGS) ../../common/src/aixv41Kludges.C

ifdef GCC_2_95
CXXFLAGS	+= -fno-exceptions
TFLAGS		+= -fno-exceptions
endif

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
