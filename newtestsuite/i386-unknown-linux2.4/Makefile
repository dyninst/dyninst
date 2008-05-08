#
# $Id: Makefile,v 1.3 2008/05/08 20:53:16 cooksey Exp $
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

# Tests 1, 2, and 12 use libdl. It is only added to their link lines
# as we must also test the harder-to-bootstrap general case (mutatee
# without the libdl)
MUTATEE_USE_LIBDL_SELECTIVELY = true
MUTATEE_LIBS =
MUTATEE_LDFLAGS_GNU += -Wl,-export-dynamic
MUTATEE_LDFLAGS_NATIVE += -Wl,-export-dynamic

COMMA=,
MUTATEE_G77_FFLAGS += -Di386_unknown_linux2_0 -DF77 -fno-second-underscore -g
MUTATEE_G77_CFLAGS += $(filter-out -Wl$(COMMA)-export-dynamic, $(MUTATEE_CFLAGS_GNU)) 
MUTATEE_G77_CFLAGS += -DF77 -DFortran -g $(MUTATEE_FFLAGS) -c -I../src
TEST1_FORTRAN_CSOURCE = test1.mutateeFortC.c

MUTATOR_LIBS += -liberty
MUTATEE_CFLAGS_NATIVE 	= $(UNIFIED_DEF) -g -I../src
MUTATEE_CXXFLAGS_NATIVE = $(UNIFIED_DEF) -g -I../src

# Definitions used for test1 assembly
# TEST1_AS = gcc -c
# TEST1_ASFLAGS = $(MUTATEE_CFLAGS_GNU)
# CALL35_1_SRC = call35_1_x86_linux.s

# Definition used for test6 assembly
# NASM ?= nasm
M_nasm ?= $(NASM)
# TEST6_AS_GNU = $(NASM)
# TEST6_ASFLAGS_GNU = -f elf -dPLATFORM=$(PLATFORM)
# TEST6_AS_SRC = test6LS-x86.asm
# TEST6_AS_OBJ_BASE = $(basename $(TEST6_AS_SRC))

TESTLIB_FLAGS = -fpic -shared -g -Wl,-export-dynamic -Wl,-ldl

# see discussion in make.module.tmpl for the format of this variable
MUTATEE_TEST9_EXTRA_LIB = 9.-L./ 9.-lInstMe

LIBS_LIBTESTSUITE = -ldl

STRIP_SO = strip -g

# .PHONY: force_default
# force_default: make.vars.gen all
# force_default: solo_mutatees # FIXME This is a temporary entry

include ../make.module.tmpl

# These rules should build the specification-generated files if necessary
# TO_SPEC = ../src/specification
# TO_SRC ?= ../src

# PROLOG_FILES = spec.pl test.pl makegen.pl util.pl

# $(PROLOG_FILES): %: $(TO_SPEC)/%
# 	cp -f $(TO_SPEC)/$* .

# PYTHONPATH += $(TO_SPEC)

# tuples: $(PROLOG_FILES)
# 	gprolog --entry-goal "['spec.pl']" \
# 		--entry-goal "test_init('$(PLATFORM)')" \
# 		--entry-goal "write_tuples('tuples', '$(PLATFORM)')" \
# 		--entry-goal "halt"

# make.vars.gen: $(PROLOG_FILES) $(TO_SPEC)/makemake.py $(TO_SPEC)/tuples.py tuples
# 	env PYTHONPATH=$(PYTHONPATH) python -c "import makemake; makemake.write_make_vars_gen('make.vars.gen', 'tuples')"

# make.solo_mutatee.gen: $(PROLOG_FILES) $(TO_SPEC)/makemake.py $(TO_SPEC)/tuples.py tuples
# 	env PYTHONPATH=$(PYTHONPATH) python -c "import makemake; makemake.write_make_solo_mutatee_gen('make.solo_mutatee.gen', 'tuples')"

# include make.solo_mutatee.gen
