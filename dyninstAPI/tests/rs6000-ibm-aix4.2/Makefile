#
# $Id: Makefile,v 1.18 2001/08/30 18:17:15 beyerj Exp $
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

MUTATEE_FFLAGS += -U -WF,-Drs6000_ibm_aix4_1 -qnolm -qfixed -w -g
MUTATEE_FLINKFLAGS += -g -bgcbypass:3 -ldl -lgcc -L/usr/local/lib/gcc-lib/powerpc-ibm-aix4.3.2.0/2.95.3

NATIVE_CC = xlc
NATIVE_CXX = xlC

GNU_CC = gcc
GNU_CXX = g++

NATIVE_FC = xlf90
COMMA=,

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

ifeq ($(NATIVE_FC), $(MUTATEE_FC))
COMPILER = xlf90
endif

MUTATEE_CFLAGS += -DCOMPILER='"$(COMPILER)"'
LDFLAGS += -lbsd

ifeq ($(NATIVE_FC), $(MUTATEE_FC))

# FORTRAN_BASE = test1.mutateeFort, etc.
FORTRAN_BASE = $(TARGET2:_$(NATIVE_FC)=Fort)

$(TARGET2): $(FORTRAN_BASE)_$(NATIVE_FC).o $(FORTRAN_BASE)C_$(NATIVE_FC).o
	-$(NATIVE_FC) $(FORTRAN_BASE)_$(NATIVE_FC).o $(FORTRAN_BASE)C_$(NATIVE_FC).o $(MUTATEE_FLINKFLAGS) -o $(TARGET2)
 
$(FORTRAN_BASE)_$(NATIVE_FC).o: ../src/$(FORTRAN_BASE).F
	-$(NATIVE_FC) $(MUTATEE_FFLAGS) -c -o $(FORTRAN_BASE)_$(NATIVE_FC).o ../src/$(FORTRAN_BASE).F
 
$(FORTRAN_BASE)C_$(NATIVE_FC).o: ../src/$(TARGET2:_$(NATIVE_FC)=).c
	-$(MUTATEE_CC) $(filter-out -Wl$(COMMA)-bgcbypass:3, $(MUTATEE_CFLAGS)) -fno-implicit-templates -W -Wall -fno-squangle -DFortran -c -o $(FORTRAN_BASE)C_$(NATIVE_FC).o ../src/$(TARGET2:_$(NATIVE_FC)=).c
else
$(TARGET2): $(MUTATEE_SRC)
	-$(MUTATEE_CC) -g $(MUTATEE_CFLAGS) -Wl,-bnoobjreorder \
	$(MUTATEE_SRC) -o $(TARGET2)
endif

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
