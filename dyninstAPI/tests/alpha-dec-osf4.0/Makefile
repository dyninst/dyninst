# $Id: Makefile,v 1.15 2001/06/04 18:42:25 bernat Exp $
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

CXXFLAGS += -mcpu=ev56 
CFLAGS += -g

NATIVE_CC = cc
NATIVE_CXX = cxx
GNU_CXX = g++

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

COMPILER = $(shell $(MUTATEE_CC) -v 2>&1 | grep version | head -1)

MUTATEE_CFLAGS = $(MODCFLAGS) -g
ifeq ($(MUTATEE_CC),$(NATIVE_CC))
COMPILER = $(shell $(MUTATEE_CC) -V 2>&1 | head -1)
MUTATEE_CFLAGS = $(ARCH_DEF) -g
endif
ifeq ($(MUTATEE_CC),$(NATIVE_CXX))
COMPILER = $(shell $(MUTATEE_CC) -V 2>&1 | head -1)
MUTATEE_CFLAGS = -x cxx $(ARCH_DEF) -g
endif

MUTATEE_CFLAGS += -DCOMPILER='"$(COMPILER)"'

ifdef GCC_2_95
MUTATEE_CFLAGS += -gcoff
endif

$(TARGET2): $(MUTATEE_SRC)
	-$(MUTATEE_CC) $(MUTATEE_CFLAGS) $(MUTATEE_SRC) -o $(TARGET2) $(MUTLIBS)

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
