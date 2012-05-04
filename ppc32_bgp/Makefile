#
# $Id: Makefile,v 1.1 2008/05/08 20:53:29 cooksey Exp $
#

# Define any symbols needed to invoke configuration changes in make.config
TO_CORE	= 	../..
NO_OPT_FLAG=true
NO_IMPLICIT_TEMPLATES = true
USES_DWARF_DEBUG = true
USES_LIBELF = true
DEBUG_FLAG = -g

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.
include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:

#if M_G77 is defined, g77 mutatees will be built
M_G77 = g77

#GNU_WARNING_FLAGS = -W -Wall
TESTDRIVER_LDFLAGS = -liberty
MUTATOR_CXXFLAGS += $(GNU_WARNING_FLAGS)
MUTATEE_CFLAGS_GNU 	= $(UNIFIED_DEF) $(GNU_WARNING_FLAGS)
MUTATEE_CXXFLAGS_GNU 	= $(UNIFIED_DEF) $(GNU_WARNING_FLAGS)

# Tests 1, 2, and 12 use libdl. It is only added to their link lines
# as we must also test the harder-to-bootstrap general case (mutatee
# without the libdl)
MUTATEE_USE_LIBDL_SELECTIVELY = true
MUTATEE_LIBS =

MUTATEE_LDFLAGS_GNU    += -ldl $(LDFLAGS)
MUTATEE_LDFLAGS_NATIVE += -ldl $(LDFLAGS)

COMMA=,
MUTATEE_G77_FFLAGS += -Dppc64_linux -DF77 -fno-second-underscore -g
MUTATEE_G77_FFLAGS += $(F77FLAGS)
MUTATEE_G77_CFLAGS += $(filter-out -Wl$(COMMA)-export-dynamic, $(MUTATEE_CFLAGS_GNU)) 
MUTATEE_G77_CFLAGS += -DF77 -DFortran -g $(MUTATEE_FFLAGS) -c
MUTATEE_G77_CFLAGS += $(CFLAGS)
TEST1_FORTRAN_CSOURCE = test1.mutateeFortC.c

MUTATEE_G77_LDFLAGS += $(LDFLAGS)

MUTATEE_CFLAGS_NATIVE 	= $(UNIFIED_DEF) -g
MUTATEE_CXXFLAGS_NATIVE = $(UNIFIED_DEF) -g

# Definitions used for test1 assembly
TEST1_AS = bgxlc -c
TEST1_ASFLAGS = $(MUTATEE_CFLAGS_GNU)
#CALL35_1_SRC = call35_1_x86_linux.s

# Definition used for test6 assembly
NASM = $(GCC)
TEST6_AS_GNU = $(NASM)
#TEST6_ASFLAGS_GNU = -f elf -dPLATFORM=$(PLATFORM)
TEST6_ASFLAGS_GNU = $(CFLAGS) -c
TEST6_AS_SRC = test6LS-powerpc.S
TEST6_AS_OBJ_BASE = $(basename $(TEST6_AS_SRC))

TESTLIB_FLAGS = -qpic -qmkshrobj -g -Wl,-export-dynamic -Wl,-ldl
TESTLIB_FLAGS += $(LDFLAGS)
TESTSUITE_FLAGS += $(LDFLAGS)
MUTATOR_LDFLAGS += $(LDFLAGS)

# see discussion in make.module.tmpl for the format of this variable
MUTATEE_TEST9_EXTRA_LIB = 9.-L./ 9.-lInstMe

LIBS_LIBTESTSUITE = -ldl

STRIP_SO = strip -g

TESTLIB_CC = bgxlc

include ../make.module.tmpl

# Some ppc32_linux distributions require _XOPEN_SOURCE to be defined as 600
# to get the functions/types we require
CFLAGS   += -D_XOPEN_SOURCE=600
CXXFLAGS += -D_XOPEN_SOURCE=600
F77FLAGS += -D_XOPEN_SOURCE=600
