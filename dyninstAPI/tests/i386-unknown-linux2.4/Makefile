#
# $Id: Makefile,v 1.15 2001/06/04 18:42:27 bernat Exp $
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

ifdef DETACH_ON_THE_FLY
USES_MUTATEE_LIBS = true
endif
MUTATEE_LIBS = -ldl

include ../make.module.tmpl

ifeq (pg,$(findstring pg,$(MUTATEE_CC)))
COMPILER = $(shell $(MUTATEE_CC) -V 2>/dev/null | grep '^pg' | head -1 \
                                                | sed -e 's/ /_/g')
else
COMPILER = $(shell $(MUTATEE_CC) -v 2>&1 | grep version | head -1)
endif

MUTATEE_CFLAGS = -g $(ARCH_DEF)

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

$(TARGET2): $(MUTATEE_SRC)
	$(MUTATEE_CC) $(MUTATEE_CFLAGS) $(MUTATEE_SRC) -o $@ $(MUTLIBS)

ifeq (test1,$(findstring test1,$(TARGET2)))
MUTATEE_SRC	+=	../src/call35_1_x86_linux.s
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
