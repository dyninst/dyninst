#
# $Id: Makefile,v 1.11 2005/05/13 09:17:21 jaw Exp $
#

# Define any symbols needed to invoke configuration changes in make.config
TO_CORE	= 	../../..
NO_OPT_FLAG=true
INSTANTIATE_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:

# Uncomment M_G77 (at yr own risk) to do g77 build
#M_G77 = g77

NATIVE_FC = xlf90
NATIVE_FORTRAN_CC = gcc
NATIVE_CC = xlc
NATIVE_CXX = xlC

COMMA=,

CXXFLAGS += -DUSES_DYNAMIC_INF_HEAP -g -DAIX5 -Wa,-many
CFLAGS += -DUSES_DYNAMIC_INF_HEAP -g -DAIX5 -Wa,-many

MUTATEE_NATIVE_FFLAGS += -U -WF,-Drs6000_ibm_aix4_1 -qnolm -qfixed -w -g -WF,-DAIX5
MUTATEE_NATIVE_FC_LDFLAGS += -g -bgcbypass:3 -ldl `$(GCC) -print-libgcc-file-name`
TEST1_FORTRAN_CSOURCE = test1.mutateeFortC.c

TEST6_AS_GNU = as
TEST6_ASFLAGS_GNU = 
TEST6_ASSUFFIX = s
TEST6_AS_SRC = test6LS-power.s

c1 = $(shell lslpp -l 'vacpp.cmp.C' | grep COMMITTED | head -1)
c2 = $(shell lslpp -l 'vacpp.cmp.core' | grep COMMITTED | head -1)
c3 = $(shell lslpp -l 'xlC.C' | grep COMMITTED | head -1)
versionArray = $(word 2,$(c1))
versionArray += $(word 2,$(c2))
versionArray += $(word 2,$(c3))
versionArray += unknown
COMPILERT_NATIVE_CC = $(NATIVE_CC) version $(firstword $(versionArray))
COMPILERT_NATIVE_CXX = $(COMPILERT_NATIVE_CC)

MUTATEE_CFLAGS_NATIVE = $(UNIFIED_DEF) 
MUTATEE_CFLAGS_NATIVE += -D__XLC__ -qnoinline -qnooptimize 
MUTATEE_CXXFLAGS_NATIVE = $(MUTATEE_CFLAGS_NATIVE) -+
MUTATEE_CFLAGS_NATIVE += -qcpluscmt
MUTATEE_NATIVE_FC_CFLAGS = $(MUTATEE_CFLAGS_NATIVE)

MUTATOR_CXXFLAGS = $(UNIFIED_DEF) 

MUTATEE_CFLAGS_GNU = $(UNIFIED_DEF)
MUTATEE_LDFLAGS_GNU += -Wl,-bgcbypass:3  -Wl,-bnoobjreorder 
MUTATEE_CXXFLAGS_GNU = $(MUTATEE_CFLAGS_GNU)

MUTATEE_LDFLAGS_NATIVE += -bnoobjreorder -bgcbypass:3 -g -DAIX5 -Drs6000_ibm_aix4_1 -D__XLC__
 
CFLAGS_TESTLIBS = -I../src -I../../../dyninstAPI_RT/h
TESTLIB_FLAGS = -bM:SRC -bexpall -bnoentry -berok
MAKE_SHARED_LIB = ld

include ../make.module.tmpl


