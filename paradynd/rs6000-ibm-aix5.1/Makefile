#
# $Id: Makefile,v 1.10 2004/03/05 16:51:01 bernat Exp $
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

AIX_PROC = true

include ../../make.config 

TEMP_INLINE_DEFINES =
TFLAGS += -fno-inline-functions
LDFLAGS += -Xlinker -bbigtoc

# Now make any necessary architecture specific changes to variables:

SRCS =		../../dyninstAPI/src/aix.C \
		../../dyninstAPI/src/sol_proc.C \
		../../dyninstAPI/src/aixMT.C \
		../../dyninstAPI/src/aixDL.C \
		../../dyninstAPI/src/dynamiclinking.C \
		../../paradynd/src/init-aix.C \
		../../paradynd/src/proc-aix.C \
		../../dyninstAPI/src/inst-power.C \
		../../dyninstAPI/src/parseStab.C \
		../../dyninstAPI/src/inst-aix.C \
		../../dyninstAPI/src/Object-xcoff.C \
		../../dyninstAPI/src/FunctionExpansionRecord.C \
		../../dyninstAPI/src/unix.C \
		../../dyninstAPI/src/InstrucIter-power.C \
      ../../dyninstAPI/src/writeBackXCOFF.C \
		../../dyninstAPI/src/syscall-solproc.C


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
