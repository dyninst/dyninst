#
# Makefile for test programs on Sparc Solaris 2.4
#
# $Id: Makefile,v 1.1 2005/09/29 20:37:22 bpellin Exp $
#

# Define any symbols needed to invoke configuration changes in make.config
TO_CORE	= 	../..
NO_OPT_FLAG=true
NO_IMPLICIT_TEMPLATES = true

MUTATEE_LIBS = -ldl -lrt

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.
include $(TO_CORE)/make.config 

DEBUG_FLAG = -g
NATIVE_DEBUG_FLAG = -g -xs

# Now make any necessary architecture specific changes to variables:

MUTATEE_G77_FFLAGS += -DF77 -g -Dsparc_sun_solaris2_4 -fno-second-underscore  $(filter-out -Wl$(COMMA)-export-dynamic, $(MUTATEE_CFLAGS)) -W -Wall  -DFortran -DCOMPILER='"g77"' -c
TEST1_FORTRAN_CSOURCE = test1.mutateeFortC.c
MUTATEE_G77_CFLAGS += $(MUTATEE_G77_FFLAGS)
LIBGCCLOC ?= /usr/local/gcc-2.95.3-cv/lib/gcc-lib/sparc-sun-solaris2.8/2.95.3
LIBGCCLOC_ALT ?= .
MUTATEE_NATIVE_FC_FLAGS += -L$(LIBGCCLOC) -L(LIBGCCLOC_ALT) -ldl -lgcc

M_G77 = g77

# Native compilers are set here, but will be unset later if not found
NATIVE_CC = cc
NATIVE_CXX = CC
NATIVE_FC = f90
NATIVE_FORTRAN_CC = cc

MUTATEE_NATIVE_FFLAGS += -Dsparc_sun_solaris2_4 $(NATIVE_DEBUG_FLAG) -DCOMPILER='"$(NATIVE_FC)"'
MUTATEE_NATIVE_FC_CFLAGS += $(MUTATEE_NATIVE_FFLAGS)

# skip the builds with native (WorkShop) compilers if they're not available
ifeq (,$(findstring Forte,$(shell $(NATIVE_CC) -V 2>&1)))
ifeq (,$(findstring WorkShop,$(shell $(NATIVE_CC) -V 2>&1)))
ifeq (,$(findstring Sun C ,$(shell $(NATIVE_CC) -V 2>&1)))
NATIVE_CC =
#$(warning No cc)
endif
endif
endif

# One may have cc without CC...
ifeq (,$(findstring Forte,$(shell $(NATIVE_CXX) -V 2>&1)))
ifeq (,$(findstring WorkShop,$(shell $(NATIVE_CXX) -V 2>&1)))
ifeq (,$(findstring Sun C++,$(shell $(NATIVE_CXX) -V 2>&1))) 
NATIVE_CXX =
#$(warning No CC)
endif
endif
endif

# One may have cc without f90...
ifeq (,$(findstring Forte,$(shell $(NATIVE_FC) -V 2>&1)))
ifeq (,$(findstring WorkShop,$(shell $(NATIVE_FC) -V 2>&1)))
ifeq (,$(findstring Sun Fortran,$(shell $(NATIVE_FC) -V 2>&1)))
NATIVE_FC =
#$(warning No F)
endif
endif
endif

TEST6_AS_GNU = gcc -c 
TEST6_AS_SRC = test6LS-sparc.S
TEST6_AS_OBJ_BASE = $(basename $(TEST6_AS_SRC))
TEST6_ASFLAGS_GNU = -P -Wa,-xarch=v8plus

CALL35_1_SRC = call35_1_sparc_solaris.s

# see make.module.tmpl for a description of how this var is formatted
MUTATEE_TEST9_EXTRA_LIB = 9.-L./ 9.-lInstMe

ifdef NATIVE_CC
COMPILERT_NATIVE = $(shell $(NATIVE_CC) -V 2>&1 | head -1)
MUTATEE_CFLAGS_NATIVE = $(UNIFIED_DEF) $(NATIVE_DEBUG_FLAG) -Wa,-xarch=v8plus
endif

ifdef NATIVE_CXX
COMPILERT_NATIVE_CXX = $(shell $(NATIVE_CXX) -V 2>&1 | head -1)
MUTATEE_CXXFLAGS_NATIVE = $(UNIFIED_DEF) $(NATIVE_DEBUG_FLAG) -instances=static
endif

MUTATEE_CFLAGS_GNU = $(MODCFLAGS) $(DEBUG_FLAG)
MUTATOR_CXXFLAGS += -W -Wall

TESTLIB_FLAGS = -shared -fpic

LIBS_LIBTESTSUITE = -ldl

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl
MUTATOR_LDFLAGS +=  $(SYSLIBS) -lelf -lstdc++ -lsocket -lrt -liberty
