# Define any symbols needed to invoke configuration changes in make.config
TO_CORE	= 	../..
NO_OPT_FLAG=true
NO_IMPLICIT_TEMPLATES = true
DEBUG_FLAG = -g 

CC = gcc
CXX = g++
PLATFORM_CXXFLAGS = 
# Include standard make configuration stuff that applies to everything
# in the paradyn tree.
include $(TO_CORE)/make.config.local
include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:

#if M_ABI is defined, secondary ABI specific mutatees will be built
ifndef SKIP_BUILD_RTLIB_32
	M_ABI = m32
	M_ABI_FLAG = -m32 -Di386_unknown_linux2_4 -Dm32_test
endif

GNU_WARNING_FLAGS = -Wall
MUTATOR_CXXFLAGS += $(GNU_WARNING_FLAGS) 
MUTATEE_CFLAGS_GNU 	= $(UNIFIED_DEF) $(GNU_WARNING_FLAGS)
MUTATEE_CXXFLAGS_GNU 	= $(UNIFIED_DEF) $(GNU_WARNING_FLAGS)

# Tests 1, 2, and 12 use libdl. It is only added to their link lines
# as we must also test the harder-to-bootstrap general case (mutatee
# without the libdl)
MUTATEE_USE_LIBDL_SELECTIVELY = true
MUTATEE_LIBS =
MUTATEE_LDFLAGS_NATIVE += -Wl,-E

COMMA=,
MUTATEE_G77_FFLAGS += -Di386_unknown_linux2_0 -DF77 -fno-second-underscore -g
MUTATEE_G77_CFLAGS += $(filter-out -Wl$(COMMA)-export-dynamic, $(MUTATEE_CFLAGS_GNU)) 
MUTATEE_G77_CFLAGS += -DF77 -DFortran -g $(MUTATEE_FFLAGS)  -fno-implicit-templates -c
TEST1_FORTRAN_CSOURCE = test1.mutateeFortC.c

MUTATEE_CFLAGS_NATIVE 	= $(UNIFIED_DEF)
MUTATEE_CXXFLAGS_NATIVE = $(UNIFIED_DEF)

# Definitions used for test1 assembly
TEST1_AS = gcc -c
TEST1_ASFLAGS = $(MUTATEE_CFLAGS_GNU)
CALL35_1_SRC = call35_1_x86_64_linux.s

# Definition used for test6 assembly
TEST6_AS_GNU = as
TEST6_ASFLAGS_GNU =
TEST6_AS_SRC = test6LS-x86_64.s
TEST6_AS_OBJ_BASE = $(basename $(TEST6_AS_SRC))

TESTLIB_FLAGS = -fpic -shared -g -Wl,-export-dynamic -Wl,-ldl

# see discussion in make.module.tmpl for the format of this variable
MUTATEE_TEST9_EXTRA_LIB = 9.-L./ 9.-lInstMe

LIBS_LIBTESTSUITE = -ldl


include ../make.module.tmpl

