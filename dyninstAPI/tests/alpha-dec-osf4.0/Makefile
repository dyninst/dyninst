# $Id: Makefile,v 1.10 2000/06/20 21:49:44 paradyn Exp $
#
# Makefile for dyninstAPI test programs on Compaq Alpha
#

# Define any symbols needed to invoke configuration changes in make.config
TO_CORE	= 	../../..
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

MUTATEE_SRC = ../src/$(TARGET2:_$(MUTATEE_CC)=.c)

MUTATEE_CFLAGS = $(MODCFLAGS) -g
ifeq ($(MUTATEE_CC),$(NATIVE_CC))
MUTATEE_CFLAGS = $(ARCH_DEF) -g
endif
ifeq ($(MUTATEE_CC),$(NATIVE_CXX))
MUTATEE_CFLAGS = -x cxx $(ARCH_DEF) -g
endif

ifdef GCC_2_95
MUTATEE_CFLAGS += -gcoff
endif

ifdef USES_LIBDYNINSTRT_SO
$(TARGET2): $(MUTATEE_SRC)
	$(MUTATEE_CC) $(MUTATEE_CFLAGS) $(MUTATEE_SRC) -o $(TARGET2) $(MUTLIBS)
else
$(TARGET2): $(MUTATEE_SRC)
	$(MUTATEE_CC) $(MUTATEE_CFLAGS) $(MUTATEE_SRC) -o $(TARGET2) \
		$(MUTLIBS) $(DYNINSTAPI_RT_LIB)
endif

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
