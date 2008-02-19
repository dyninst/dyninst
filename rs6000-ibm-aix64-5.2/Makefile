#
# $Id: Makefile,v 1.1 2008/02/19 13:52:27 rchen Exp $
#

# Define any symbols needed to invoke configuration changes in make.config
TO_CORE	= 	../..
NO_OPT_FLAG=true
INSTANTIATE_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:

# Undefine ARCH_FLAGS to allow M_ABI to work correctly
ARCH_FLAGS =

#if M_ABI is defined, secondary ABI specific mutatees will be built
M_ABI = 32
M_ABI_FLAG = -maix32
M_ABI_NATIVE_LDFLAG = -b32
STD_ABI_FLAG = -maix64 -Drs6000_ibm_aix64
STD_NATIVE_ABI_FLAG = -q64 -Drs6000_ibm_aix64
STD_NATIVE_ABI_FFLAG = -q64
STD_NATIVE_ABI_LDFLAG = -b64

# Uncomment M_G77 (at yr own risk) to do g77 build
M_G77 = g77

NATIVE_FC = xlf90
NATIVE_FORTRAN_CC = xlc
NATIVE_CC = xlc
NATIVE_CXX = xlC
NATIVE_CXX_OMP = xlC_r

COMMA=,

CXXFLAGS += -g -DAIX5 -Wa,-many
CFLAGS += -g -DAIX5 -Wa,-many

MUTATEE_NATIVE_FFLAGS += -U -WF,-Drs6000_ibm_aix4_1 -qnolm -qfixed -w -g -WF,-DAIX5,-DXLF
MUTATEE_NATIVE_FC_LDFLAGS += -g -bgcbypass:3 -ldl `$(GCC) -print-libgcc-file-name`
MUTATEE_G77_FFLAGS += -Drs6000_ibm_aix4_1 -DAIX5 -DF77 -fno-second-underscore -g
MUTATEE_G77_CFLAGS += $(filter-out -Wl$(COMMA)-export-dynamic, $(MUTATEE_CFLAGS_GNU))
MUTATEE_G77_CFLAGS += -DF77 -DFortran -g $(MUTATEE_FFLAGS) -c
TEST1_FORTRAN_CSOURCE = test1.mutateeFortC.c

TEST6_AS_GNU = as
TEST6_ASFLAGS_GNU = -a64
TEST6_AS_SRC = test6LS-power.s
TEST6_AS_OBJ_BASE = $(basename $(TEST6_AS_SRC))

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
MUTATEE_NATIVE_FC_CFLAGS = $(MUTATEE_CFLAGS_NATIVE) -DXLF
MUTATEE_NATIVE_CXX_CFLAGS = -qsmp=omp 

MUTATOR_CXXFLAGS = $(UNIFIED_DEF) 

MUTATEE_CFLAGS_GNU = $(UNIFIED_DEF)
MUTATEE_LDFLAGS_GNU += -Wl,-bgcbypass:3  -Wl,-bnoobjreorder 
MUTATEE_CXXFLAGS_GNU = $(MUTATEE_CFLAGS_GNU)

MUTATEE_LDFLAGS_NATIVE += -bnoobjreorder -bgcbypass:3 -g -DAIX5 -Drs6000_ibm_aix4_1 -D__XLC__
MUTATEE_NATIVE_CXX_OMP_LDFLAGS = -ldl -lpthread -lxlsmp
 
CFLAGS_TESTLIBS = -I../src -I$(TO_CORE)/dyninstAPI_RT/h
TESTLIB_FLAGS = -bM:SRC -bexpall -bnoentry -berok -bnoobjreorder
LIBTEST12_FLAGS = $(TESTLIB_FLAGS) -lc -L$(TO_CORE)/dyninstAPI_RT/$(PLATFORM) -ldyninstAPI_RT -lpthread
MAKE_SHARED_LIB = ld

TESTSUITE_FLAGS += -Wl,-bexpall   

LIBS_LIBTESTSUITE = -ldl -lpthread
USE_LIBTESTSUITE_EXT_A = true
STRIP_SO = strip

include ../make.module.tmpl
