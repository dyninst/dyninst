#
# Makefile for test programs on Sparc Solaris 2.4
#
# $Id: Makefile,v 1.1 2001/06/04 16:24:14 schendel Exp $
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

GNU_CXX = g++
NATIVE_CC = cc
NATIVE_CXX = CC

# skip the builds with native (WorkShop) compilers if they're not available
ifeq (,$(findstring WorkShop,$(shell $(NATIVE_CC) -V 2>&1)))
NATIVE_CC =
#else
#ifneq (,$(findstring "not found",$(shell which $(NATIVE_CXX) 2>&1)))
NATIVE_CXX =
#endif
endif

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

COMPILER = $(shell $(MUTATEE_CC) -v 2>&1 | grep version | head -1)

ifeq ($(MUTATEE_CC),$(NATIVE_CC))
COMPILER = $(shell $(MUTATEE_CC) -V 2>&1 | head -1)
MUTATEE_CFLAGS = $(ARCH_DEF) -g
else
MUTATEE_CFLAGS = $(MODCFLAGS) -g
endif

ifeq ($(MUTATEE_CC),$(NATIVE_CXX))
# using "-instances=static" or "-instances=global" flag allows the template 
# instantiations being in the current object file and gives them static and
# global linkage respectively
CXXFLAGS        +=  -instances=static
COMPILER = $(shell $(MUTATEE_CC) -V 2>&1 | head -1)
endif

MUTATEE_CFLAGS += -DCOMPILER='"$(COMPILER)"'

ifdef USES_LIBDYNINSTRT_SO
$(TARGET2):  $(MUTATEE_SRC)
	$(MUTATEE_CC) $(MUTATEE_CFLAGS) $(MUTATEE_SRC) -o $@ $(MUTLIBS)
else
DYNINSTAPI_RT_LIB=$(TO_CORE)/$(LIBRARY_DEST)/libdyninstAPI_RT.o
$(TARGET2):  $(MUTATEE_SRC)
        $(MUTATEE_CC) $(MUTATEE_CFLAGS) $(DYNINSTAPI_RT_LIB) $(MUTATEE_SRC) -o $@.$(CC) $(MUTLIBS)
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
