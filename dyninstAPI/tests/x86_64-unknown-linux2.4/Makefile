#
# $Id: Makefile,v 1.4 2005/07/06 18:28:32 rchen Exp $
#

# Define any symbols needed to invoke configuration changes in make.config
TO_CORE	= 	../../..
NO_OPT_FLAG=true
NO_IMPLICIT_TEMPLATES = true
USES_DWARF_DEBUG = true
USES_LIBELF = true
DEBUG_FLAG = -g

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.
include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:

#if M_ABI is defined, secondary ABI specific mutatees will be built
M_ABI = m32
M_ABI_FLAG = -m32

#if M_G77 is defined, g77 mutatees will be built
#M_G77 = g77

# there is no "native" or OS-provided compiler for Linux!
# we use the Portland Group compilers, as that's what we've currently got
# (in future we probably also want to test with the KAI/Intel compilers)
ifdef PGI
NATIVE_CC = pgcc
NATIVE_CXX = pgCC
COMPILERT_NATIVE_CC = $(shell $(NATIVE_CC) -V 2>/dev/null | grep '^pg' | head -1)
COMPILERT_NATIVE_CXX = $(shell $(NATIVE_CXX) -V 2>/dev/null | grep '^pg' | head -1)
endif

GNU_WARNING_FLAGS = -W -Wall
MUTATOR_CXXFLAGS += $(GNU_WARNING_FLAGS)
MUTATEE_CFLAGS_GNU 	= $(UNIFIED_DEF) $(GNU_WARNING_FLAGS)
MUTATEE_CXXFLAGS_GNU 	= $(UNIFIED_DEF) $(GNU_WARNING_FLAGS)
MUTATEE_LIBS = -Wl,-ldl
MUTATEE_LDFLAGS_GNU += -Wl,-export-dynamic

COMMA=,
MUTATEE_G77_FFLAGS += -Di386_unknown_linux2_0 -DF77 -fno-second-underscore -g
MUTATEE_G77_CFLAGS += $(filter-out -Wl$(COMMA)-export-dynamic, $(MUTATEE_CFLAGS_GNU)) 
MUTATEE_G77_CFLAGS += -DF77 -DFortran -g $(MUTATEE_FFLAGS)  -fno-implicit-templates -c
TEST1_FORTRAN_CSOURCE = test1.mutateeFortC.c

MUTATOR_LIBS += -liberty
MUTATEE_CFLAGS_NATIVE 	= $(UNIFIED_DEF)
MUTATEE_CXXFLAGS_NATIVE = $(UNIFIED_DEF)

# Definitions used for test1 assembly
TEST1_AS = gcc -c
TEST1_ASFLAGS = $(MUTATEE_CFLAGS_GNU)
CALL35_1_SRC = call35_1_x86_64_linux.s
CALL35_1_SRC_ABI = call35_1_x86_linux.s

# Definition used for test6 assembly
TEST6_AS_GNU = nasm
TEST6_ASFLAGS_GNU = -f elf -dPLATFORM=$(PLATFORM)
TEST6_AS_SRC = test6LS-x86.asm
TEST6_AS_OBJ_BASE = test6LS-x86_64
TEST6_AS_OBJ_BASE_ASM = $(basename $(TEST6_AS_OBJ_BASE))

TESTLIB_FLAGS = -fpic -shared -g -Wl,-export-dynamic -Wl,-ldl

# see discussion in make.module.tmpl for the format of this variable
MUTATEE_TEST9_EXTRA_LIB = 9.-L./ 9.-lInstMe
MUTATEE_TEST9_EXTRA_LIB_ABI = 9.-L./ 9.-lInstMe_$(M_ABI)

include ../make.module.tmpl

