#
# $Id: Makefile,v 1.1 2002/02/11 22:01:26 tlmiller Exp $
#

# Define any symbols needed to invoke configuration changes in make.config
TO_CORE	= 	../../..
NO_OPT_FLAG=true
NO_IMPLICIT_TEMPLATES = true

DEBUG_FLAG = -gstabs+

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:

CXXFLAGS += 
CFLAGS +=

MUTATEE_FFLAGS += -Dia64_unknown_linux2_4 -DF77 -fno-second-underscore -g
MUTATEE_FLINKFLAGS += -g -ldl

SYSLIBS += -lelf -ldl
GNU_CXX = g++
GNU_CC = gcc
# there is no "native" or OS-provided compiler for Linux!
# we use the Portland Group compilers, as that's what we've currently got
# (in future we probably also want to test with the KAI/Intel compilers)
ifdef PGI
NATIVE_CC = pgcc
NATIVE_CXX = pgCC
endif

NATIVE_FC = g77
COMMA=,

MINI_PLATFORM = ia64
TEST6_ASFLAGS = 
TEST6_ASSUFFIX = s

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

ifdef DETACH_ON_THE_FLY
USES_MUTATEE_LIBS = true
endif
MUTATEE_LIBS = -ldl

include ../make.module.tmpl

ifeq (pg,$(findstring pg,$(MUTATEE_CC)))
COMPILERT = $(shell $(MUTATEE_CC) -V 2>/dev/null | grep '^pg' | head -1)
else
COMPILERT = $(shell $(MUTATEE_CC) -v 2>&1 | grep version | head -1)
endif

MUTATEE_CFLAGS = -g $(ARCH_DEF)

EMPTY:=
BLANK:=$(EMPTY) $(EMPTY)
LPAREN:=(
RPAREN:=)
COMPILER1 = $(subst $(BLANK),_,$(COMPILERT))
COMPILER2 = $(subst $(LPAREN),_,$(COMPILER1))
COMPILER = $(subst $(RPAREN),_,$(COMPILER2))

ifeq ($(NATIVE_FC), $(MUTATEE_FC))
COMPILER = g77
endif

MUTATEE_CFLAGS += -DCOMPILER='"$(COMPILER)"'

ifdef DETACH_ON_THE_FLY
MUTATEE_CFLAGS += -DDETACH_ON_THE_FLY
endif

ifeq (test1,$(findstring test1,$(TARGET2)))
# test1.mutatee loads libtestA.so & libtestB.so for tests 21 & 22
# and these libraries need access to global variables in test1.mutatee
#MUTATEE_CFLAGS += -rdynamic
MUTATEE_CFLAGS += -Wl,-export-dynamic
endif

ifeq ($(NATIVE_FC), $(MUTATEE_FC))

# FORTRAN_BASE = test1.mutateeFort, etc.
FORTRAN_BASE = $(TARGET2:_$(NATIVE_FC)=Fort)

$(TARGET2): $(FORTRAN_BASE)_$(NATIVE_FC).o $(FORTRAN_BASE)C_$(NATIVE_FC).o
	-$(NATIVE_FC) $(FORTRAN_BASE)_$(NATIVE_FC).o $(FORTRAN_BASE)C_$(NATIVE_FC).o $(MUTATEE_FLINKFLAGS) -o $(TARGET2)

$(FORTRAN_BASE)_$(NATIVE_FC).o: ../src/$(FORTRAN_BASE).F
	-$(NATIVE_FC) $(MUTATEE_FFLAGS) -c -o $(FORTRAN_BASE)_$(NATIVE_FC).o ../src/$(FORTRAN_BASE).F

$(FORTRAN_BASE)C_$(NATIVE_FC).o: ../src/$(TARGET2:_$(NATIVE_FC)=).c
	-$(MUTATEE_CC) $(filter-out -Wl$(COMMA)-export-dynamic, $(MUTATEE_CFLAGS)) -fno-implicit-templates -W -Wall $(FORT_SQGL_FLAG) -DFortran -c -o $(FORTRAN_BASE)C_$(NATIVE_FC).o ../src/$(TARGET2:_$(NATIVE_FC)=).c
else
$(TARGET2): $(MUTATEE_SRC) $(MUTATEE_OBJS)
	$(MUTATEE_CC) $(MUTATEE_CFLAGS) $(MUTATEE_SRC) $(MUTATEE_OBJS) -o $@ $(MUTLIBS)
endif

ifeq (test1,$(findstring test1,$(TARGET2)))
MUTATEE_SRC	+=	../src/call35_1_ia64_linux.s
endif

libtestA.o:
	$(CC) -g -fpic -c ../src/libtestA.c

libtestB.o:
	$(CC) -g -fpic -c ../src/libtestB.c

libtestA.so: libtestA.o
	$(CC) -shared -o libtestA.so libtestA.o

libtestB.so: libtestB.o
	$(CC) -shared -o libtestB.so libtestB.o

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include $(TO_CORE)/make.program.tmpl



