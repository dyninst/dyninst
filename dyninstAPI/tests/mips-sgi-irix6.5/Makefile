#
# $Id: Makefile,v 1.1 2000/03/30 23:45:14 wylie Exp $
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
NATIVE_CC	= cc

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

MUTATEE_SRC = ../src/$(TARGET2:_$(MUTATEE_CC)=.c)

ifeq ($(MUTATEE_CC),$(NATIVE_CC))
MUTATEE_CFLAGS = -64 $(MODCFLAGS) -g
else
MUTATEE_CFLAGS = -mabi=64 $(ARCH_DEF) -g
endif

# 64-bit mutatee
$(TARGET2): $(MUTATEE_SRC)
	$(MUTATEE_CC) $(MUTATEE_CFLAGS) -g $(MUTATEE_SRC) -o $(TARGET2)

libtestA.o: ../src/libtestA.c
	$(CC) -c $< -o $@

libtestB.o: ../src/libtestB.c
	$(CC) -c $< -o $@

libtestA.so: libtestA.o
	$(CC) -shared -o $@ $<

libtestB.so: libtestB.o
	$(CC) -shared -o $@ $<

# 32-bit mutatee
TARGET3   = $(TARGET2)_n32
XTARGET3  = $(TARGET2)

$(TARGET3): $(MUTATEE_SRC)
	$(CC2) $(CFLAGS) -g $(MUTATEE_SRC) -o $(TARGET3)

$(TARGET3).o: ../src/$(XTARGET3).c
	$(CC2) $(CFLAGS) -c $< -o $@

aTest: $(TARGET3)

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
TARGETS += test1.mutatee_gcc_n32  test2.mutatee_gcc_n32 test3.mutatee_gcc_n32 \
           test4a.mutatee_gcc_n32 test4b.mutatee_gcc_n32
TARGETS += libtestA_n32.so libtestB_n32.so
endif


# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include $(TO_CORE)/make.program.tmpl
