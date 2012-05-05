#
# $Id: Makefile,v 1.4 2008/05/30 20:29:28 cooksey Exp $
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
include $(TO_CORE)/make.config.local
include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:

#if M_G77 is defined, g77 mutatees will be built
M_G77 = g77

NATIVE_CC = gcc
NATIVE_CXX = g++ 

GNU_WARNING_FLAGS = -W -Wall
MUTATOR_CXXFLAGS += $(GNU_WARNING_FLAGS)
MUTATEE_CFLAGS_GNU 	= $(UNIFIED_DEF) $(GNU_WARNING_FLAGS)
MUTATEE_CXXFLAGS_GNU 	= $(UNIFIED_DEF) $(GNU_WARNING_FLAGS)

# Tests 1, 2, and 12 use libdl. It is only added to their link lines
# as we must also test the harder-to-bootstrap general case (mutatee
# without the libdl)
MUTATEE_USE_LIBDL_SELECTIVELY = true
MUTATEE_LIBS =

COMMA=,
MUTATEE_G77_FFLAGS += -Damd64_unknown_freebsd7_0 -DF77 -fno-second-underscore -g
MUTATEE_G77_CFLAGS += $(filter-out -Wl$(COMMA)-export-dynamic, $(MUTATEE_CFLAGS_GNU)) 
MUTATEE_G77_CFLAGS += -DF77 -DFortran -g $(MUTATEE_FFLAGS) -c -I../src
TEST1_FORTRAN_CSOURCE = test1.mutateeFortC.c

MUTATEE_CFLAGS_NATIVE 	= $(UNIFIED_DEF) -g -I../src
MUTATEE_CXXFLAGS_NATIVE = $(UNIFIED_DEF) -g -I../src

# Definitions used for test1 assembly
# TEST1_AS = gcc -c
# TEST1_ASFLAGS = $(MUTATEE_CFLAGS_GNU)
# CALL35_1_SRC = call35_1_x86_linux.s

TESTLIB_FLAGS = -fpic -shared -g -Wl,-export-dynamic

ifeq ($(GNU_DEMANGLER),)
TESTDRIVER_LDFLAGS += -L$(LIBIBERTY_LIB) -liberty
endif

# see discussion in make.module.tmpl for the format of this variable
MUTATEE_TEST9_EXTRA_LIB = 9.-L./ 9.-lInstMe

LIBS_LIBTESTSUITE = -lstdc++

STRIP_SO = strip -g

# .PHONY: force_default
# force_default: make.vars.gen all
# force_default: solo_mutatees # FIXME This is a temporary entry

# FreeBSD doesn't have purify which is defined in make.module.tmpl
M_GCC = $(GCC)

include ../make.module.tmpl
