#
# $Id: Makefile,v 1.1 2000/03/31 16:53:01 wylie Exp $
#

# Define any symbols needed to invoke configuration changes in make.config
TO_CORE	= 	../../..
NO_OPT_FLAG=true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:

MODCC = g++
CXX   = g++

CXXFLAGS +=
CFLAGS +=

NATIVE_CC = cc

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

DYNINSTAPI_RT_LIB=$(TO_CORE)/$(LIBRARY_DEST)/libdyninstAPI_RT.o

# replace testN.mutatee_compiler with ../src/testN.mutatatee.c
MUTATEE_SRC = ../src/$(TARGET2:_$(MUTATEE_CC)=.c)
MUTATEE_OBJ = $(TARGET2:_$(MUTATEE_CC)=.o)

ifeq ($(MUTATEE_CC),$(NATIVE_CC))
MUTATEE_CFLAGS = $(ARCH_DEF) -bnso -bI:/lib/syscalls.exp -bkeepfile:$(MUTATEE_OBJ) -bgcbypass:3
else
MUTATEE_CFLAGS = $(MODCFLAGS) -static -Wl,-bgcbypass:3 
endif

$(TARGET2): $(MUTATEE_SRC)
	$(MUTATEE_CC) $(MUTATEE_CFLAGS) -Wl,-bnoobjreorder \
	-Wl,-bE:$(TO_CORE)/$(LIBRARY_DEST)/DYNINSTAPI_RT_EXPORTS \
	$(DYNINSTAPI_RT_LIB) $(MUTATEE_SRC) -o $(TARGET2)

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include $(TO_CORE)/make.program.tmpl
