IFLAGS = -I$(DYNINST_ROOT)/include -I$(DYNINST_ROOT)/dyninst/DDG/h
LFLAGS = -L${DYNINST_ROOT}/$(PLATFORM)/lib -ldyninstAPI -lsymtabAPI -linstructionAPI -ldepGraphAPI
CFLAGS += -g

all : test_DDG testAll test coverage

test : test.c
	gcc -o test test.c

test_DDG : test_DDG.C
	g++ -o test_DDG test_DDG.C $(IFLAGS) $(CFLAGS) $(LFLAGS)

testAll : testAll.C
	g++ -o testAll testAll.C $(IFLAGS) $(CFLAGS) $(LFLAGS)

coverage : coverage.C
	g++ -o coverage coverage.C $(IFLAGS) $(CFLAGS) $(LFLAGS)


clean :
	rm -f test test_DDG testAll coverage *~
