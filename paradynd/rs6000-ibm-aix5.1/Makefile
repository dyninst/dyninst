#
# $Id: Makefile,v 1.14 2006/07/07 00:01:10 jaw Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_LIBERTY = true

# Enable shared memory sampling
USES_SHM_SAMPLING = true
NO_IMPLICIT_TEMPLATES = true

# Turn on my (bernat's) debugging hack
# DEBUG = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

TEMP_INLINE_DEFINES =
TFLAGS += -fno-inline-functions
LDFLAGS += -Xlinker -bbigtoc

# Now make any necessary architecture specific changes to variables:

SRCS =		../../paradynd/src/init-aix.C \
		../../paradynd/src/proc-aix.C 

ifdef GCC_2_95
CXXFLAGS	+= -fno-exceptions
TFLAGS		+= -fno-exceptions
endif 

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

PD_NO_DEPENDS = true

include ../../make.program.tmpl

fastInferiorHeapHKs.o:
	$(CXX) -c -mpowerpc $(CPPFLAGS) $(CXXFLAGS) $< -o $@
