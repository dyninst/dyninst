#
# $Id: Makefile,v 1.17 2001/08/29 23:25:30 hollings Exp $
#

# Define any symbols needed to invoke configuration changes in make.config
TO_CORE	= 	../../..
NO_OPT_FLAG=true
NO_IMPLICIT_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:

MODCC = g++
CXX   = g++

CXXFLAGS += -DUSES_DYNAMIC_INF_HEAP -g
CFLAGS += -DUSES_DYNAMIC_INF_HEAP -g

NATIVE_CC = xlc
NATIVE_CXX = xlC

GNU_CC = gcc
GNU_CXX = g++

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# replace testN.mutatee_compiler with ../src/testN.mutatatee.c
MUTATEE_OBJ = $(TARGET2:_$(MUTATEE_CC)=.o)

ifeq (xl,$(findstring xl,$(MUTATEE_CC)))
c1 = $(shell lslpp -l 'vacpp.cmp.C' | grep COMMITTED | head -1)
c2 = $(shell lslpp -l 'xlC.C' | grep COMMITTED | head -1)
versionArray = $(word 2,$(c1))
versionArray += $(word 2,$(c2))
versionArray += unknown
COMPILERT = $(MUTATEE_CC) version $(firstword $(versionArray))
MUTATEE_CFLAGS = $(ARCH_DEF) -bkeepfile:$(MUTATEE_OBJ)
ifeq ($(MUTATEE_CC),$(NATIVE_CXX))
MUTATEE_CFLAGS += -+
endif
else
COMPILERT = $(shell $(MUTATEE_CC) -v 2>&1 | grep version | head -1)
MUTATEE_CFLAGS = $(MODCFLAGS) -Wl,-bgcbypass:3 
endif

EMPTY:=
BLANK:=$(EMPTY) $(EMPTY)
LPAREN:=(
RPAREN:=)
COMPILER1 = $(subst $(BLANK),_,$(COMPILERT))
COMPILER2 = $(subst $(LPAREN),_,$(COMPILER1))
COMPILER = $(subst $(RPAREN),_,$(COMPILER2))
MUTATEE_CFLAGS += -DCOMPILER='"$(COMPILER)"'
LDFLAGS += -lbsd

$(TARGET2): $(MUTATEE_SRC)
	-$(MUTATEE_CC) -g $(MUTATEE_CFLAGS) -Wl,-bnoobjreorder \
	$(MUTATEE_SRC) -o $(TARGET2)

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include $(TO_CORE)/make.program.tmpl

libtestA.o:
	$(CC) -g -c ../src/libtestA.c
libtestB.o:
	$(CC) -g -c ../src/libtestB.c
libtestA.so: libtestA.o
	ld -bM:SRE -bnoentry -o libtestA.so libtestA.o -bexpall -berok
libtestB.so: libtestB.o
	ld -bM:SRE -bnoentry -o libtestB.so libtestB.o -bexpall -berok

