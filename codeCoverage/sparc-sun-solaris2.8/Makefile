# Define any symbols needed to invoke configuration changes in make.config

#UNCOMMON_INSTALL = true

USES_TCLTK = true
NO_IMPLICIT_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS =		../src/FCAllBlocks.C \
		../src/FCUseDominator.C \
		../src/FunctionCoverage.C \
		../src/CCPreInstrument.C \
		../src/CCOnDemandInstrument.C \
		../src/CodeCoverage.C 

IFLAGS		+= -I/usr/imports/include

CFLAGS		+= -DNO_MDL_PRINT -DUSES_DYNAMIC_INF_HEAP -g -O0
CXXFLAGS	+= -DNO_MDL_PRINT -DUSES_DYNAMIC_INF_HEAP -g -O0 -DUSES_ELF32_ONLY 
TFLAGS		+= -DUSES_ELF32_ONLY 

TCLTK_LIBS	?= -ltk8.0 -ltcl
SYSLIBS 	= -lrt -lelf -L/user/gaburici/opt/tcltk-8.3.4/lib $(TCLTK_LIBS) -lpthread

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

ifndef AUTO_TEMPLATES
SRCS         += ../src/templates.C
endif

templates.o: ../src/templates.C
	$(CXX) $(TFLAGS) -c ../src/templates.C

LIBS 	+= -ldyninstAPI
include ../../make.program.tmpl
