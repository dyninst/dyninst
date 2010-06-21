# Define any symbols needed to invoke configuration changes in make.config

#DEBUG_FLAG = -g
AUTO_TEMPLATES= true
#NO_IMPLICIT_TEMPLATES = true
USES_BOOST = true

# For debugging.
#NO_OPT_FLAG = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

TO_CORE = ../..

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:
#MYFLAGS         += -g

CXXFLAGS        += $(MYFLAGS)
CFLAGS          += $(MYFLAGS)

SRCS =  ../src/InstructionSource-sparc.C \
        ../src/InstrucIter-xplat.C\
        ../src/InstrucIter-sparc.C\
		../src/IA_InstrucIter.C

#LIBS    += -lgcc

TARGET = libparseAPI.so
# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
