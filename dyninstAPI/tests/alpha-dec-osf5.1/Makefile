# $Id: Makefile,v 1.5 2004/03/05 16:51:04 bernat Exp $
#
# Makefile for dyninstAPI test programs on Tru64 Alpha
#

# Define any symbols needed to invoke configuration changes in make.config
TO_CORE	= 	../../..
NO_IMPLICIT_TEMPLATES=true
NO_OPT_FLAG=true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:

CFLAGS += -g
MUTATEE_FFLAGS += -Dalpha_dec_osf4_0 -g

NATIVE_CC = cc
NATIVE_CXX = cxx
M_GXX = g++
M_GCC = gcc
NATIVE_FC = f90
COMMA=,

MINI_PLATFORM = alpha
TEST6_ASFLAGS = -cpp
TEST6_ASSUFFIX = s

ifeq ($(NATIVE_FC), $(MUTATEE_FC))

COMPILER = f90
# FORTRAN_BASE = test1.mutateeFort, etc.
FORTRAN_BASE = $(TARGET2:_$(NATIVE_FC)=Fort)
FORTRAN_BASE = test1.mutateeFort

$(TARGET2): $(FORTRAN_BASE)_$(NATIVE_FC).o $(FORTRAN_BASE)C_$(NATIVE_FC).o test1.mutateeCommon.o mutatee_util.o
	-$(MUTATEE_CC) -c ../src/mutatee_util.c
	-$(NATIVE_FC) -g -nofor_main $(FORTRAN_BASE)_$(NATIVE_FC).o $(FORTRAN_BASE)C_$(NATIVE_FC).o test1.mutateeCommon.o mutatee_util.o $(MUTATEE_FLINKFLAGS) -o $(TARGET2)

$(FORTRAN_BASE)_$(NATIVE_FC).o: ../src/$(FORTRAN_BASE).F
	-$(NATIVE_FC) $(MUTATEE_FFLAGS) -c -o $(FORTRAN_BASE)_$(NATIVE_FC).o ../src/$(FORTRAN_BASE).F

$(FORTRAN_BASE)C_$(NATIVE_FC).o: ../src/$(TARGET2:_$(NATIVE_FC)=).c
	-$(MUTATEE_CC) $(filter-out -Wl$(COMMA)-bgcbypass:3, $(MUTATEE_CFLAGS)) -fno-implicit-templates -W -Wall $(FORT_SQGL_FLAG) -DFortran -c -o $(FORTRAN_BASE)C_$(NATIVE_FC).o ../src/$(FORTRAN_BASE)C.c
else
$(TARGET2): $(MUTATEE_SRC) $(MUTATEE_OBJS)
	-$(MUTATEE_CC) -g $(MUTATEE_CFLAGS) \
	$(MUTATEE_SRC) $(MUTATEE_OBJS) -o $(TARGET2)
endif

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

COMPILERT = $(shell $(MUTATEE_CC) -v 2>&1 | grep version | head -1)

MUTATEE_CFLAGS = $(MODCFLAGS) -g
ifeq ($(MUTATEE_CC),$(NATIVE_CC))
COMPILERT = $(shell $(MUTATEE_CC) -V 2>&1 | head -1)
MUTATEE_CFLAGS = $(UNIFIED_DEF) -g
endif
ifeq ($(MUTATEE_CC),$(NATIVE_CXX))
COMPILERT = $(shell $(MUTATEE_CC) -V 2>&1 | head -1)
MUTATEE_CFLAGS = -x cxx $(UNIFIED_DEF) -g -D__USE_STD_IOSTREAM
endif

EMPTY:=
BLANK:=$(EMPTY) $(EMPTY)
COMPILER = $(subst $(BLANK),_,$(COMPILERT))
MUTATEE_CFLAGS += -DCOMPILER='"$(COMPILER)"'

#$(TARGET2): $(MUTATEE_SRC) $(MUTATEE_OBJS)
#	-$(MUTATEE_CC) $(MUTATEE_CFLAGS) $(MUTATEE_SRC) $(MUTATEE_OBJS) -o $(TARGET2) $(MUTLIBS)

libtestA.o:
	$(CC) -g -fpic -c ../src/libtestA.c

libtestB.o:
	$(CC) -fpic -c ../src/libtestB.c

libtestA.so: libtestA.o
	$(CC) -g -shared -o libtestA.so libtestA.o

libtestB.so: libtestB.o
	$(CC) -shared -o libtestB.so libtestB.o

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include $(TO_CORE)/make.program.tmpl
