#
# $Id: Makefile,v 1.2 2005/10/22 22:11:48 bpellin Exp $
#

# Define any symbols needed to invoke configuration changes in make.config
TO_CORE	= 	../..
NO_OPT_FLAG=true
NO_IMPLICIT_TEMPLATES = true
USES_DWARF_DEBUG = true
USES_UNWIND_LIB = true

DEBUG_FLAG = -g

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:

CXXFLAGS += -Wno-deprecated
CFLAGS += -Wno-deprecated

M_GXX = $(GXX)
M_GCC = $(GCC)
M_G77 = g77

GNU_WARNING_FLAGS = -W -Wall
MUTATOR_CXXFLAGS += $(GNU_WARNING_FLAGS)
MUTATEE_CFLAGS_GNU      = $(UNIFIED_DEF) $(GNU_WARNING_FLAGS) -Wl,-export-dynamic
MUTATEE_CXXFLAGS_GNU    = $(UNIFIED_DEF) $(GNU_WARNING_FLAGS) -Wl,-export-dynamic
MUTATEE_LIBS =  -ldl
MUTATEE_LDFLAGS_GNU += -Wl,-export-dynamic

COMMA=,
MUTATEE_G77_FFLAGS += -Dia64_unknown_linux2_4 -DF77 -fno-second-underscore -g
MUTATEE_G77_CFLAGS += $(UNIFIED_DEF) -DCOMPILER='"g77"' -Wl,-export-dynamic
MUTATEE_G77_CFLAGS +=  -DFortran -g  -fno-implicit-templates -c
MUTATEE_G77_LDFLAGS += -Wl,-export-dynamic

#  test1_fortran_csource is probably not correct...  kept here to make
# sure we can build (but not run) fortran mutatees.  
# should be test1.mutateeFortC.c, I think.
TEST1_FORTRAN_CSOURCE = test1.mutatee.c

MUTATEE_TEST9_LIBS_LIST = libInstMe.so libLoadMe.so
MUTATEE_TEST9_LIBS = -L. -lInstMe

MUTATOR_LIBS += -liberty -ldl
MUTATEE_CFLAGS_NATIVE   = $(UNIFIED_DEF)
MUTATEE_CXXFLAGS_NATIVE = $(UNIFIED_DEF)

# Definitions used for test1 assembly
TEST1_AS = gcc -c
TEST1_ASFLAGS = $(MUTATEE_CFLAGS_GNU)
CALL35_1_SRC = call35_1.c

# Definition used for test6 assembly
TEST6_AS_GNU = as
TEST6_ASFLAGS_GNU = 
TEST6_AS_SRC = test6LS-ia64.s
TEST6_AS_OBJ_BASE = $(basename $(TEST6_AS_SRC))

TESTLIB_FLAGS = -fpic -shared -g -Wl,-export-dynamic -Wl,-ldl

LIBS_LIBTESTSUITE = -ldl

STRIP_SO = strip -g

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

