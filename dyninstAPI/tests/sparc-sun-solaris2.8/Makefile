#
# Makefile for test programs on Sparc Solaris 2.4
#
# $Id: Makefile,v 1.12 2001/11/01 05:00:26 gaburici Exp $
#

# Define any symbols needed to invoke configuration changes in make.config
TO_CORE	= 	../../..
NO_OPT_FLAG=true
NO_IMPLICIT_TEMPLATES = true

APP_PURE_OPTIONS = 

USES_MUTATEE_LIBS = true
MUTATEE_LIBS = -ldl

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:

CXXFLAGS += -DUSES_DYNAMIC_INF_HEAP
CFLAGS += -g -DUSES_DYNAMIC_INF_HEAP
SYSLIBS += -lelf -g -lstdc++ 

MUTATEE_FFLAGS += -g -Dsparc_sun_solaris2_4 -xs

#the _ALT PATH is in case the gcc library can't be found in the first path
ifdef GCC_2_95
LIBGCCLOC = /p/paradyn/packages/gcc-2.95.3/lib/gcc-lib/PLATFORM/2.95.3
# Not using the gcc-2.95.3 on tau (in stow); it is broken...
LIBGCCLOC_ALT = /usr/local/gcc-2.95.3-cv/lib/gcc-lib/sparc-sun-solaris2.8/2.95.3
else
LIBGCCLOC = /p/paradyn/packages/gcc-3/gcc-3.0.1/lib/gcc-lib/PLATFORM/3.0.1
LIBGCCLOC_ALT = /usr/local/gcc-2.95.3-cv/lib/gcc-lib/sparc-sun-solaris2.8/2.95.3
endif
MUTATEE_FLINKFLAGS += -g -ldl -lgcc -L$(LIBGCCLOC) -L$(LIBGCCLOC_ALT)

GNU_CC = gcc
GNU_CXX = g++

# define these only if not invoked recursively, otherwise they're set
ifndef MUTATEE_CC
export NATIVE_CC = cc
export NATIVE_CXX = CC
export NATIVE_FC = f90

# skip the builds with native (WorkShop) compilers if they're not available
ifeq (,$(findstring WorkShop,$(shell $(NATIVE_CC) -V 2>&1)))
NATIVE_CC =
#$(warning No cc)
endif

# One may have cc without CC...
ifeq (,$(findstring WorkShop,$(shell $(NATIVE_CXX) -V 2>&1)))
NATIVE_CXX =
#$(warning No CC)
endif

# One may have cc without f90...
ifeq (,$(findstring WorkShop,$(shell $(NATIVE_FC) -V 2>&1)))
NATIVE_FC =
#$(warning No F)
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
MUTATEE_CFLAGS = $(ARCH_DEF) -g -xs -Wa,-xarch=v8plus
else
MUTATEE_CFLAGS = $(MODCFLAGS) -g
endif

ifeq ($(MUTATEE_CC),$(NATIVE_CXX))
# using "-instances=static" or "-instances=global" flag allows the template 
# instantiations being in the current object file and gives them static and
# global linkage respectively
CXXFLAGS        +=  -instances=static
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

ifdef NATIVE_FC
ifeq ($(NATIVE_FC),$(MUTATEE_FC))

COMPILER = f90

# FORTRAN_BASE = test1.mutateeFort, etc.
FORTRAN_BASE = $(TARGET2:_$(NATIVE_FC)=Fort)

$(TARGET2): $(FORTRAN_BASE)_$(NATIVE_FC).o $(FORTRAN_BASE)C_$(NATIVE_FC).o
	$(NATIVE_FC) -xs $(MUTATEE_FFLAGS) ../src/$(FORTRAN_BASE).F $(FORTRAN_BASE)C_$(NATIVE_FC).o test1.mutateeCommon.o $(MUTATEE_FLINKFLAGS) -o $(TARGET2)

$(FORTRAN_BASE)_$(NATIVE_FC).o:
	-$(NATIVE_FC) $(MUTATEE_FFLAGS) -c -o $(FORTRAN_BASE)_$(NATIVE_FC).o ../src/$(FORTRAN_BASE).F

$(FORTRAN_BASE)C_$(NATIVE_FC).o:
	$(MUTATEE_CC) $(MUTATEE_CFLAGS) -fno-implicit-templates -W -Wall $(FORT_SQGL_FLAG) -DFortran -c -o $(FORTRAN_BASE)C_$(NATIVE_FC).o ../src/$(FORTRAN_BASE)C.c

else
$(TARGET2):  $(MUTATEE_SRC)
	$(MUTATEE_CC) $(MUTATEE_CFLAGS) $(MUTATEE_SRC) $(MUTATEE_OBJS) -o $@ $(MUTLIBS) 
endif
endif

ifeq (test1,$(findstring test1,$(TARGET2)))
MUTATEE_SRC     +=      ../src/call35_1_sparc_solaris.s
endif


#MODCC = purecov -best-effort g++

libtestA.o:
	$(CC) -fpic -c ../src/libtestA.c

libtestB.o:
	$(CC) -fpic -c ../src/libtestB.c

libtestA.so: libtestA.o
	$(CC) -shared -o libtestA.so libtestA.o

libtestB.so: libtestB.o
	$(CC) -shared -o libtestB.so libtestB.o

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include $(TO_CORE)/make.program.tmpl
