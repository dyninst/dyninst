#
# $Id: Makefile,v 1.17 2003/07/18 15:44:07 schendel Exp $
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
M_GXX		= g++
M_GCC           = gcc
NATIVE_CC	= cc
NATIVE_CXX	= CC
CFLAGS   += -LANG:std
CXXFLAGS += -LANG:std
TFLAGS   += -LANG:std
KFLAGS   += -LANG:std
LDFLAGS  += -LANG:std

MINI_PLATFORM = mips
TEST6_ASFLAGS = 
TEST6_ASSUFFIX = s

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

COMPILERT = $(shell $(MUTATEE_CC) -v 2>&1 | grep version | head -1)

ifeq ($(MUTATEE_CC),$(NATIVE_CC)) 
COMPILERT = $(shell $(MUTATEE_CC) -version 2>&1 | head -1)
MABI = -
MUTATEE_CFLAGS = $(MODCFLAGS) -g
endif

ifeq ($(MUTATEE_CC),$(NATIVE_CXX))
COMPILERT = $(shell $(MUTATEE_CC) -version 2>&1 | head -1)
MABI = -
MUTATEE_CFLAGS = $(MODCFLAGS) -g
endif

ifeq ($(MUTATEE_CC), $(M_GXX))
MABI = -mabi=
MUTATEE_CFLAGS = $(ARCH_DEF) -g
endif

ifeq ($(MUTATEE_CC), $(M_GCC))
MABI = -mabi=
MUTATEE_CFLAGS = $(ARCH_DEF) -g
endif

EMPTY:=
BLANK:=$(EMPTY) $(EMPTY)
COMPILER = $(subst $(BLANK),_,$(COMPILERT))
MUTATEE_CFLAGS += -DCOMPILER='"$(COMPILER)"'

# 64-bit mutatee
$(TARGET2): $(MUTATEE_SRC) $(MUTATEE_OBJS)
	$(MUTATEE_CC) $(MABI)64 $(MUTATEE_CFLAGS) $(MUTATEE_SRC) -o $@ $(MUTLIBS)

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
#XTARGET3  = $(TARGET2)

# VG(11/08/01): Hack to make a _n32 assembler file.
# All this 32/64 bit thing should be moved to the template...
# ifeq (test6.mutatee_gcc, $(findstring test6.mutatee_gcc,$(TARGET2)))
ifeq (test6, $(findstring test6,$(TARGET2)))

TEST6_AS_OBJ_32	:= $(TEST6_AS_OBJ:%.o=%_n32.o)
MUTATEE_SRC_32	:= $(MUTATEE_SRC:$(TEST6_AS_OBJ)=$(TEST6_AS_OBJ_32))

test6.mutatee_gcc_n32:	$(TEST6_AS_OBJ_32)

$(TEST6_AS_OBJ_32):	../src/$(TEST6_AS_SRC)
	as -n32 $(TEST6_ASFLAGS) -o $@ $<
else
MUTATEE_SRC_32 := $(MUTATEE_SRC)
# $(warning mutatee_src_32 is $(MUTATEE_SRC_32))
endif

$(TARGET3): $(MUTATEE_SRC_32)
	$(MUTATEE_CC) $(MABI)n32 $(MUTATEE_CFLAGS) $(MUTATEE_SRC_32) -o $@ $(MUTLIBS)

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

# VG(11/08/01): This is now handled by the code above I believe...

# 32-bit targets
#ifndef TARGET
#ifdef NATIVE_CC
#TARGETS += test1.mutatee_cc_n32  test2.mutatee_cc_n32 test3.mutatee_cc_n32 \
#           test4a.mutatee_cc_n32 test4b.mutatee_cc_n32
#endif
#
#ifdef GNU_CXX
#TARGETS += test1.mutatee_g++_n32  test2.mutatee_g++_n32 test3.mutatee_g++_n32 \
#           test4a.mutatee_g++_n32 test4b.mutatee_g++_n32 test5.mutatee_g++_n32
#endif
#
#ifdef NATIVE_CXX 
#TARGETS += test1.mutatee_CC_n32  test2.mutatee_CC_n32 test3.mutatee_CC_n32 \
#           test4a.mutatee_CC_n32 test4b.mutatee_CC_n32 test5.mutatee_CC_n32
#endif
#
#TARGETS += test1.mutatee_gcc_n32  test2.mutatee_gcc_n32 test3.mutatee_gcc_n32 \
#           test4a.mutatee_gcc_n32 test4b.mutatee_gcc_n32 test5.mutatee_g++_n32
#TARGETS += libtestA_n32.so libtestB_n32.so
#endif


# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include $(TO_CORE)/make.program.tmpl

clean::
	$(RM) *.o *.[hC] test?* *.so

