#
# $Id: Makefile,v 1.30 2003/04/24 15:09:23 schendel Exp $
#

# Define any symbols needed to invoke configuration changes in make.config
TO_CORE	= 	../../..
NO_OPT_FLAG=true
NO_IMPLICIT_TEMPLATES = true
USES_DWARF_DEBUG = true

DEBUG_FLAG = -g

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:

CXXFLAGS += 
CFLAGS +=

MUTATEE_FFLAGS += -Di386_unknown_linux2_0 -DF77 -fno-second-underscore -g
MUTATEE_FLINKFLAGS += -g -ldl

SYSLIBS += -lelf -ldl
LIBS = -L$(LIBDWARFDIR)/lib -ldwarf
M_GXX = g++
M_GCC = gcc

# there is no "native" or OS-provided compiler for Linux!
# we use the Portland Group compilers, as that's what we've currently got
# (in future we probably also want to test with the KAI/Intel compilers)
ifdef PGI
NATIVE_CC = pgcc
NATIVE_CXX = pgCC
endif

NATIVE_FC = g77
COMMA=,

MINI_PLATFORM = x86
TEST6_ASFLAGS = -f elf -dPLATFORM=$(PLATFORM)
TEST6_ASSUFFIX = asm

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

USES_MUTATEE_LIBS = true
MUTATEE_LIBS = -ldl

include ../make.module.tmpl

ifeq (pg,$(findstring pg,$(MUTATEE_CC)))
COMPILERT = $(shell $(MUTATEE_CC) -V 2>/dev/null | grep '^pg' | head -1)
else
COMPILERT = $(shell $(MUTATEE_CC) -v 2>&1 | grep version | head -1)
endif

# As a test, don't include debug information in the mutatees.
MUTATEE_CFLAGS = $(ARCH_DEF)

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

MUTATEE_CFLAGS += -DCOMPILER='"$(COMPILER)"' -g

ifdef DETACH_ON_THE_FLY
MUTATEE_CFLAGS += -DDETACH_ON_THE_FLY
endif

ifeq (test1,$(findstring test1,$(TARGET2)))
# test1.mutatee loads libtestA.so & libtestB.so for tests 21 & 22
# and these libraries need access to global variables in test1.mutatee
#MUTATEE_CFLAGS += -rdynamic
MUTATEE_CFLAGS += -g -Wl,-export-dynamic
endif

ifeq ($(NATIVE_FC), $(MUTATEE_FC))

# FORTRAN_BASE = test1.mutateeFort, etc.
FORTRAN_BASE = $(TARGET2:_$(NATIVE_FC)=Fort)

$(TARGET2): $(FORTRAN_BASE)_$(NATIVE_FC).o $(FORTRAN_BASE)C_$(NATIVE_FC).o test1.mutateeCommon.o mutatee_util.o
	-$(NATIVE_FC) $(FORTRAN_BASE)_$(NATIVE_FC).o $(FORTRAN_BASE)C_$(NATIVE_FC).o test1.mutateeCommon.o mutatee_util.o $(MUTATEE_FLINKFLAGS) -o $(TARGET2)

$(FORTRAN_BASE)_$(NATIVE_FC).o: ../src/$(FORTRAN_BASE).F
	-$(NATIVE_FC) $(MUTATEE_FFLAGS) -c -o $(FORTRAN_BASE)_$(NATIVE_FC).o ../src/$(FORTRAN_BASE).F

$(FORTRAN_BASE)C_$(NATIVE_FC).o: ../src/$(TARGET2:_$(NATIVE_FC)=).c
	-$(MUTATEE_CC) $(filter-out -Wl$(COMMA)-export-dynamic, $(MUTATEE_CFLAGS)) -fno-implicit-templates -W -Wall $(FORT_SQGL_FLAG) -DFortran -c -o $(FORTRAN_BASE)C_$(NATIVE_FC).o ../src/$(TARGET2:_$(NATIVE_FC)=).c
else
$(TARGET2): $(MUTATEE_SRC) $(MUTATEE_OBJS)
	$(MUTATEE_CC) $(MUTATEE_CFLAGS) $(MUTATEE_SRC) $(MUTATEE_OBJS) -o $@ $(MUTLIBS)
endif

ifeq (test1,$(findstring test1,$(TARGET2)))
MUTATEE_SRC	+=	../src/call35_1_x86_linux.s
endif

# (remove debug information for testing)
libtestA.o:
	$(CC) -fpic -c ../src/libtestA.c

# (remove debug information for testing)
libtestB.o:
	$(CC) -fpic -c ../src/libtestB.c

libtestA.so: libtestA.o
	$(CC) -shared -o libtestA.so libtestA.o

libtestB.so: libtestB.o
	$(CC) -shared -o libtestB.so libtestB.o

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include $(TO_CORE)/make.program.tmpl

