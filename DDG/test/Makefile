DYNINST_ROOT = /p/paradyn/development/bernat/builds
IFLAGS = -I$(DYNINST_ROOT)/include -I$(DYNINST_ROOT)/dyninst/DDG/h
CFLAGS = -g
LFLAGS = -L${DYNINST_ROOT}/$(PLATFORM)/lib -ldyninstAPI -lsymtabAPI -linstructionAPI -lDDG


all : test_DDG test coverage

test : 
	gcc -o test test.c

test_DDG :
	g++ -o test_DDG test_DDG.C $(IFLAGS) $(CFLAGS) $(LFLAGS)

coverage :
	g++ -o coverage coverage.C $(IFLAGS) $(CFLAGS) $(LFLAGS)


clean :
	rm -f test test_DDG coverage *~
