# $Id: Makefile,v 1.10 2005/07/06 18:27:40 rchen Exp $
#
# Makefile for dyninstAPI test programs on Tru64 Alpha
#

# Define any symbols needed to invoke configuration changes in make.config
TO_CORE	= 	../../..
NO_IMPLICIT_TEMPLATES=true
NO_OPT_FLAG=true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:

CFLAGS += -g
CFLAGS += -pthread
CXXFLAGS += -pthread
MUTATEE_CFLAGS += -pthread
MUTATEE_FFLAGS_G77 += -Dalpha_dec_osf4_0 -g
MUTATEE_NATIVE_FFLAGS += -Dalpha_dec_osf4_0 -g
MUTATEE_NATIVE_FC_CFLAGS += $(UNIFIED_DEF) -DFortran
TEST1_FORTRAN_CSOURCE = test1.mutateeFortC.c
LDFLAGS += -lrt -lpthread

NATIVE_CC = cc
NATIVE_CXX = cxx
##  Uncomment M_G77 to trigger g77 fortran build   (at your own risk)
#M_G77 = g77
NATIVE_FC = f90
NATIVE_FORTRAN_CC = gcc
COMMA=,

COMPILERT_NATIVE_CC = $(shell $(NATIVE_CC) -V 2>&1 | head -1)
COMPILERT_NATIVE_CXX = $(shell $(NATIVE_CXX) -V 2>&1 | head -1)

MUTATEE_FFLAGS_NATIVE += -nofor_main
MUTATEE_NATIVE_FC_LDFLAGS += -nofor_main
MUTATEE_CXXFLAGS_NATIVE +=  $(UNIFIED_DEF) -g -D__USE_STD_IOSTREAM -pthread -D_REENTRANT
MUTATEE_CFLAGS_NATIVE += $(UNIFIED_DEF)  -g -pthread
MUTATEE_CFLAGS_GNU += $(UNIFIED_DEF) -pthread
MUTATEE_CXXFLAGS_GNU += $(UNIFIED_DEF) 
TESTLIB_FLAGS = -g -fpic -shared -I$(TO_CORE)/dyninstAPI_RT/h
MUTATEE_LDFLAGS_GNU += -lrt
MUTATEE_LDFLAGS_NATIVE += -lrt


TEST6_AS_GNU = as
TEST6_ASFLAGS = -cpp
TEST6_AS_SRC = test6LS-alpha.s
TEST6_AS_OBJ_BASE = $(basename $(TEST6_AS_SRC))

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.
include ../make.module.tmpl

