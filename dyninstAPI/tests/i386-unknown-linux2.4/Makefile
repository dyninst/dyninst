#
# $Id: Makefile,v 1.6 2000/05/14 19:02:52 zandy Exp $
#

# Define any symbols needed to invoke configuration changes in make.config
TO_CORE	= 	../../..
NO_OPT_FLAG=true

DEBUG_FLAG = -gstabs+

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:

CXXFLAGS += 
CFLAGS +=

SYSLIBS += -lelf -lnsl
GNU_CXX = g++
# there is no "native" or OS-provided compiler for Linux!
# we use the Portland Group compilers, as that's what we've currently got
# (in future we probably also want to test with the KAI/Intel compilers)
ifdef PGI
NATIVE_CC = pgcc
NATIVE_CXX = pgCC
endif

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

MUTATEE_SRC = ../src/$(TARGET2:_$(MUTATEE_CC)=.c)

MUTATEE_CFLAGS = -g $(ARCH_DEF)

ifdef DETACH_ON_THE_FLY
MUTATEE_CFLAGS += -DDETACH_ON_THE_FLY
endif
MUTATEE_LIBS = -ldl

ifeq ($(MUTATEE_CC),gcc)
MUTATEE_CFLAGS += -rdynamic
endif
ifeq ($(MUTATEE_CC),$(GNU_CXX))
MUTATEE_CFLAGS += -rdynamic
endif

ifdef USES_LIBDYNINSTRT_SO
$(TARGET2): $(MUTATEE_SRC)
	$(MUTATEE_CC) $(MUTATEE_CFLAGS) $(MUTATEE_SRC) -o $@ $(MUTLIBS)
else
DYNINSTAPI_RT_LIB=$(TO_CORE)/$(LIBRARY_DEST)/libdyninstAPI_RT.o

$(TARGET2): $(MUTATEE_SRC)
	$(MUTATEE_CC) $(MUTATEE_CFLAGS) $(DYNINSTAPI_RT_LIB) $(MUTATEE_SRC) \
                -o $@ $(MUTLIBS)
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
