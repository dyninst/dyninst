# $Id: Makefile,v 1.17 2001/10/26 22:45:31 hollings Exp $
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

COMPILERT = $(shell $(MUTATEE_CC) -v 2>&1 | grep version | head -1)

MUTATEE_CFLAGS = $(MODCFLAGS) -g
ifeq ($(MUTATEE_CC),$(NATIVE_CC))
COMPILERT = $(shell $(MUTATEE_CC) -V 2>&1 | head -1)
MUTATEE_CFLAGS = $(ARCH_DEF) -g
endif
ifeq ($(MUTATEE_CC),$(NATIVE_CXX))
COMPILERT = $(shell $(MUTATEE_CC) -V 2>&1 | head -1)
MUTATEE_CFLAGS = -x cxx $(ARCH_DEF) -g
endif

EMPTY:=
BLANK:=$(EMPTY) $(EMPTY)
COMPILER = $(subst $(BLANK),_,$(COMPILERT))
MUTATEE_CFLAGS += -DCOMPILER='"$(COMPILER)"'

ifdef GCC_2_95
MUTATEE_CFLAGS += -gcoff
endif

$(TARGET2): $(MUTATEE_SRC) $(MUTATEE_OBJS)
	-$(MUTATEE_CC) $(MUTATEE_CFLAGS) $(MUTATEE_SRC) $(MUTATEE_OBJS) -o $(TARGET2) $(MUTLIBS)

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
