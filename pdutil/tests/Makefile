
#
# $Log: Makefile,v $
# Revision 1.1  1994/08/17 18:26:13  markc
# Code and sample output to test new classes.
#
#
# Makefile for routines that test util libraries and classes

include ../../make.config
CFLAGS   += -DPARADYND_PVM -Wshadow -Wpointer-arith -Wcast-qual -Wconversion -Wwrite-strings -Wstrict-prototypes -Wnested-externs -Wenum-clash -Woverloaded-virtual -Winline -g -DKL_PRINT -DKYL_PRINT
PURE_CMD = purify -g++ -collector=/usr/sup/purify/collect2
# LIBDIR = -L../../../lib/$(PLATFORM) -L/p/paradyn/lib/$(PLATFORM)

all: klist cstring util

test: test_klist test_cstring test_util

test_klist: klist pure_klist
	@echo "Running purified code for klist"
	@echo "There should be no access errors, other than an uninitialized memory read from the function klist.car()"
	p_klist
	@echo "Running output test for klist"
	@echo "New output goes in klist_new_out"
	@echo "Original output is in klist_out"
	rm -f klist_new_out
	klist > klist_new_out
	diff klist_out klist_new_out

test_cstring: cstring pure_cstring
	@echo "Running purified code for cstring"
	@echo "There should be no access errors, other than an uninitialized memory read from the function klist.car()"
	p_cstring
	@echo "Running output test for cstring"
	@echo "New output goes in cstring_new_out"
	@echo "Original output is in cstring_out"
	rm -f cstring_new_out
	cstring > cstring_new_out
	diff cstring_out cstring_new_out

test_util: util pure_util
	@echo "Running purified code for util"
	@echo "There should be no access errors, other than an uninitialized memory read from the function klist.car()"
	p_util
	@echo "Running output test for util"
	@echo "New output goes in util_new_out"
	@echo "Original output is in util_out"
	rm -f util_new_out
	util > util_new_out
	diff util_out util_new_out

clean: 
	rm *pure* klist cstring util p_util p_cstring p_klist *new_out
	
klist: KListTest.C ../h/klist.h ../h/cstring.h
	$(CC) $(CFLAGS) KListTest.C -o klist

cstring: CstringTest.C ../h/cstring.h
	$(CC) $(CFLAGS) CstringTest.C -o cstring

util: utilTest.C ../h/rpcUtil.h ../src/rpcUtil.C
	$(CC) $(CFLAGS) utilTest.C $(LIBDIR) -lutil -o util
	

pure: pure_klist pure_cstring pure_util

pure_klist: 
	$(PURE_CMD) $(CC) $(CFLAGS) KListTest.C -o p_klist

pure_cstring:
	$(PURE_CMD) $(CC) $(CFLAGS) CstringTest.C -o p_cstring

pure_util:
	$(PURE_CMD) $(CC) $(CFLAGS) utilTest.C $(LIBDIR) -lutil -o p_util
