#
# $Id: Makefile,v 1.8 2000/08/20 20:43:29 wylie Exp $
#

# Define any symbols needed to invoke configuration changes in make.config
TO_CORE	= 	../../..

# optimization breaks API test #1 (required symbols disappear)
NO_OPT_FLAG = 1

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:

SYSLIBS   += -ldwarf -lelf
GNU_CXX		= g++
NATIVE_CC	= cc
NATIVE_CXX	= CC

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

COMPILER = $(shell $(MUTATEE_CC) -v 2>&1 | grep version | head -1)

ifeq ($(MUTATEE_CC),$(NATIVE_CC)) 
COMPILER = $(shell $(MUTATEE_CC) -version 2>&1 | head -1)
MABI = -
MUTATEE_CFLAGS = $(MODCFLAGS) -g
endif

ifeq ($(MUTATEE_CC),$(NATIVE_CXX))
COMPILER = $(shell $(MUTATEE_CC) -version 2>&1 | head -1)
MABI = -
MUTATEE_CFLAGS = $(MODCFLAGS) -g
endif

ifeq ($(MUTATEE_CC), $(GNU_CXX))
MABI = -mabi=
MUTATEE_CFLAGS = $(ARCH_DEF) -g
endif

ifeq ($(MUTATEE_CC), gcc)
MABI = -mabi=
MUTATEE_CFLAGS = $(ARCH_DEF) -g
endif

MUTATEE_CFLAGS += -DCOMPILER='"$(COMPILER)"'

# 64-bit mutatee
$(TARGET2): $(MUTATEE_SRC)
	$(MUTATEE_CC) $(MABI)64 $(MUTATEE_CFLAGS) $< -o $@ $(MUTLIBS)

libtestA.o: ../src/libtestA.c
	$(CC) -c $< -o $@

libtestB.o: ../src/libtestB.c
	$(CC) -c $< -o $@

libtestA.so: libtestA.o
	$(CC) -shared -o $@ $<

libtestB.so: libtestB.o
	$(CC) -shared -o $@ $<

ifdef TARGET2
# 32-bit mutatee
TARGET3   = $(TARGET2)_n32
XTARGET3  = $(TARGET2)

$(TARGET3): $(MUTATEE_SRC)
	$(MUTATEE_CC) $(MABI)n32 $(MUTATEE_CFLAGS) $< -o $@ $(MUTLIBS)

aTest: $(TARGET3)
endif

libtestA_n32.o:
	$(CC2) -c ../src/libtestA.c -o $@

libtestB_n32.o:
	$(CC2) -c ../src/libtestB.c -o $@

libtestA_n32.so: libtestA_n32.o
	$(CC2) -shared -o $@ $<

libtestB_n32.so: libtestB_n32.o
	$(CC2) -shared -o $@ $<

testlibs: libtestA_n32.so libtestB_n32.so

# 32-bit targets
ifndef TARGET
ifdef NATIVE_CC
TARGETS += test1.mutatee_cc_n32  test2.mutatee_cc_n32 test3.mutatee_cc_n32 \
           test4a.mutatee_cc_n32 test4b.mutatee_cc_n32
endif

ifdef GNU_CXX
TARGETS += test1.mutatee_g++_n32  test2.mutatee_g++_n32 test3.mutatee_g++_n32 \
           test4a.mutatee_g++_n32 test4b.mutatee_g++_n32 test5.mutatee_g++_n32
endif

ifdef NATIVE_CXX 
TARGETS += test1.mutatee_CC_n32  test2.mutatee_CC_n32 test3.mutatee_CC_n32 \
           test4a.mutatee_CC_n32 test4b.mutatee_CC_n32 test5.mutatee_CC_n32
endif

TARGETS += test1.mutatee_gcc_n32  test2.mutatee_gcc_n32 test3.mutatee_gcc_n32 \
           test4a.mutatee_gcc_n32 test4b.mutatee_gcc_n32 test5.mutatee_g++_n32
TARGETS += libtestA_n32.so libtestB_n32.so
endif


# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include $(TO_CORE)/make.program.tmpl
