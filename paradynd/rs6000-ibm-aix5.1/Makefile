#
# $Id: Makefile,v 1.2 2003/06/11 20:05:53 bernat Exp $
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

SRCS =		../../dyninstAPI/src/aix.C \
		../../dyninstAPI/src/aix-ptrace.C \
		../../dyninstAPI/src/aixDL.C \
		../../paradynd/src/init-aix.C \
		../../dyninstAPI/src/inst-power.C \
		../../dyninstAPI/src/inst-aix.C \
		../../dyninstAPI/src/Object-xcoff.C \
		../../dyninstAPI/src/FunctionExpansionRecord.C \
		../../dyninstAPI/src/unix.C

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
