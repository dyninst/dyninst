#
# Makefile for dyner and test programs on AIX.
#
# $Id: Makefile,v 1.4 2004/04/20 01:27:53 jaw Exp $
#

USES_TCLTK = true

# include ../Make.common
# DO NOT DELETE

TO_CORE =       ../..
NO_OPT_FLAG=true
INSTANTIATE_TEMPLATES = true
# NO_IMPLICIT_TEMPLATES=true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include $(TO_CORE)/make.config

# Now make any necessary architecture specific changes to variables:

CXXFLAGS += 
CFLAGS += -g

# Override these defaults by setting LDTCLFLAGS in top level make.config.local
# Recommended settig for AIX 4.3.3 w/IBM RPM supplied tcl-8.3.3 is just:
# LDTCLFLAGS := -Wl,-brtl
ifdef IBM_BPATCH_COMPAT
  LDTCLFLAGS ?= -brtl
else
  LDTCLFLAGS ?= -Xlinker -brtl
endif
LDFLAGS += $(LDTCLFLAGS)

#DYNINSTAPI_RT_LIB=$(TO_CORE)/$(LIBRARY_DEST)/libdyninstAPI_RT.o
SYSLIBS   += -lld
IFLAGS    += -I/usr/imports/include
MUTATEE_CFLAGS += -static -Wl,-bgcbypass:3 -Wl,-bnoobjreorder

#		  -Wl,-bE:$(TO_CORE)/$(LIBRARY_DEST)/DYNINSTAPI_RT_EXPORTS \
#		  $(DYNINSTAPI_RT_LIB)

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

# allow users to build tcl/tk libraries as .a's opposed to .so's since
# .so's are difficult to build on AIX.  The .so's automatically pull in the
# pthread library.
LIBS += -lpthread

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include $(TO_CORE)/make.program.tmpl
