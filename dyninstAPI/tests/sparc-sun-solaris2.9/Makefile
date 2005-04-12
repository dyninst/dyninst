#
# Makefile for test programs on Sparc Solaris 2.4
#
# $Id: Makefile,v 1.5 2005/04/12 05:14:21 jaw Exp $
#

# Define any symbols needed to invoke configuration changes in make.config
TO_CORE	= 	../../..
NO_OPT_FLAG=true
NO_IMPLICIT_TEMPLATES = true

APP_PURE_OPTIONS = 

USES_MUTATEE_LIBS = true
MUTATEE_LIBS = -ldl -lrt
MUTATEE_TEST9_LIBS_LIST = libInstMe.so libLoadMe.so
MUTATEE_TEST9_LIBS = -L. -lInstMe 

MUTATOR_TEST12_EXTRA_FLAGS = -Wa,-xarch=v8plus
# -lInstMe

# VG(12/02/01): Using stabs for dbx because gdb 5.0 cannot continue a
# mutatee stopped by a dyninst mutator (on our 24-way SunFire at
# least). BTW, -g appeard an awful number of times in this file...

# This is used by make.config for ALL compilers (unfortunately)
# It is really only for gcc
DEBUG_FLAG = -g

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.
include $(TO_CORE)/make.config 

# This is used in this file for the native compiler(s)
NATIVE_DEBUG_FLAG = -g -xs

# Now make any necessary architecture specific changes to variables:

#CXXFLAGS += -gstabs -DUSES_DYNAMIC_INF_HEAP
#CFLAGS += -gstabs -DUSES_DYNAMIC_INF_HEAP

# FIXME: This is a bad place for the debug flag...
SYSLIBS += $(DEBUG_FLAG) -lelf -lstdc++ -lsocket -lrt

MUTATEE_FFLAGS += -Dsparc_sun_solaris2_4 $(NATIVE_DEBUG_FLAG)

#the _ALT PATH is in case the gcc library can't be found in the first path
ifdef GCC_2_95
LIBGCCLOC ?= /p/paradyn/packages/gcc-2.95.3/lib/gcc-lib/PLATFORM/2.95.3
# Allowing _ALT path to be changed in make.config.local
# The default _ALT path is for UMD Solaris 8 machines.
LIBGCCLOC_ALT ?= /usr/local/gcc-2.95.3-cv/lib/gcc-lib/sparc-sun-solaris2.8/2.95.3
else
LIBGCCLOC ?= /s/gcc-3.2.2/lib/gcc-lib/sparc-sun-solaris2.8/3.2.2
LIBGCCLOC_ALT ?= /usr/local/gcc-2.95.3-cv/lib/gcc-lib/sparc-sun-solaris2.8/2.95.3
endif
MUTATEE_FLINKFLAGS += -L$(LIBGCCLOC) -L$(LIBGCCLOC_ALT) -ldl -lgcc

M_GCC = $(GCC)
M_GXX = $(GXX)

# define these only if not invoked recursively, otherwise they're set
ifndef MUTATEE_CC
export NATIVE_CC = cc
export NATIVE_CXX = CC
export NATIVE_FC = f90

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
endif

MINI_PLATFORM = sparc
TEST6_ASFLAGS = -P -xarch=v8plus
TEST6_ASSUFFIX = S

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl



COMPILERT = $(shell $(MUTATEE_CC) -v 2>&1 | grep version | head -1)

ifeq ($(MUTATEE_CC),$(NATIVE_CC))
COMPILERT = $(shell $(MUTATEE_CC) -V 2>&1 | head -1)
MUTATEE_CFLAGS = $(UNIFIED_DEF) $(NATIVE_DEBUG_FLAG) -Wa,-xarch=v8plus
else
MUTATEE_CFLAGS = $(MODCFLAGS) $(DEBUG_FLAG)
endif

ifeq ($(MUTATEE_CC),$(NATIVE_CXX))
# using "-instances=static" or "-instances=global" flag allows the template 
# instantiations being in the current object file and gives them static and
# global linkage respectively

# VG(12/02/01): Overriding these to put the right debug flag
MUTATEE_CFLAGS = $(UNIFIED_DEF) $(NATIVE_DEBUG_FLAG) -instances=static

COMPILERT = $(shell $(MUTATEE_CC) -V 2>&1 | head -1)
endif

EMPTY:=
BLANK:=$(EMPTY) $(EMPTY)
LPAREN:=(
RPAREN:=)
COMPILER1 = $(subst $(BLANK),_,$(COMPILERT))
COMPILER2 = $(subst $(LPAREN),_,$(COMPILER1))
COMPILER = $(subst $(RPAREN),_,$(COMPILER2))
MUTATEE_CFLAGS += -DCOMPILER='"$(COMPILER)"'

### C/C++ mutatee target rule ###
# MUTATEE_CC gets set even for Fortran targets. The only way
# to check for C/C++ mutatee is to look at MUTATEE_FC.
###
ifndef MUTATEE_FC
$(TARGET2):  $(MUTATEE_SRC)
	$(MUTATEE_CC) $(MUTATEE_CFLAGS) $(MUTATEE_SRC) $(MUTATEE_OBJS) -o $@ $(MUTLIBS) 
endif

### Fortran mutatee targets rules ###
# First check if we have found a native Fortran compiler.
# Then check if MUTATEE_FC matches it. For C/C++ targets
# MUTATEE_FC is undefined. NATIVE_FC may also be undefined
# if no Fortran compiler is found; the first check is useful.
###
ifdef NATIVE_FC
ifeq ($(NATIVE_FC),$(MUTATEE_FC))

COMPILER = f90

# FORTRAN_BASE = test1.mutateeFort, etc.
FORTRAN_BASE = $(TARGET2:_$(NATIVE_FC)=Fort)

# VG: Recompile the common stuff with the same C compiler (gcc) as the Fortran
# test C code. This prevents the very nasty situation where a C++ object file
# produced by another compiler (CC) is fed to the linker (which doesn't take it).
$(TARGET2): $(FORTRAN_BASE)_$(NATIVE_FC).o $(FORTRAN_BASE)C_$(NATIVE_FC).o
	$(MUTATEE_CC) $(MUTATEE_CFLAGS) -W -Wall -c -o test1.mutateeCommon.o ../src/test1.mutateeCommon.c
	$(MUTATEE_CC) $(MUTATEE_CFLAGS) -W -Wall -c -o mutatee_util.o ../src/mutatee_util.c
	$(NATIVE_FC) $(MUTATEE_FFLAGS) ../src/$(FORTRAN_BASE).F $(FORTRAN_BASE)C_$(NATIVE_FC).o test1.mutateeCommon.o mutatee_util.o $(MUTATEE_FLINKFLAGS) -o $(TARGET2)

$(FORTRAN_BASE)_$(NATIVE_FC).o:
	$(NATIVE_FC) $(MUTATEE_FFLAGS) -c -o $(FORTRAN_BASE)_$(NATIVE_FC).o ../src/$(FORTRAN_BASE).F

$(FORTRAN_BASE)C_$(NATIVE_FC).o:
	$(MUTATEE_CC) $(MUTATEE_CFLAGS) -fno-implicit-templates -W -Wall $(FORT_SQGL_FLAG) -DFortran -c -o $(FORTRAN_BASE)C_$(NATIVE_FC).o ../src/$(FORTRAN_BASE)C.c

endif
endif

ifeq (test1,$(findstring test1,$(TARGET2)))
MUTATEE_SRC     +=      ../src/call35_1_sparc_solaris.s
endif


#MODCC = purecov -best-effort g++

libtestA.o:
	$(CC) $(PICOP) -c ../src/libtestA.c

libtestB.o:
	$(CC) $(PICOP) -c ../src/libtestB.c

libtestA.so: libtestA.o
	$(CC) $(SHAREDOP) -o libtestA.so libtestA.o

libtestB.so: libtestB.o
	$(CC) $(SHAREDOP) -o libtestB.so libtestB.o


libLoadMe.o:  libLoadMe.c
	$(CC) $(PICOP) -c ../src/libLoadMe.c

libLoadMe.so: libLoadMe.o
	$(CC)  $(SHAREDOP) -z initarray=loadMe_init libLoadMe.o -o libLoadMe.so

libInstMe.o:  libInstMe.c
	$(CC) $(PICOP) -c ../src/libInstMe.c

libInstMe.so: libInstMe.o
	$(CC)  $(SHAREDOP) libInstMe.o -o libInstMe.so




# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include $(TO_CORE)/make.program.tmpl

