ROOTDIR = ../..
SRCDIR = ${ROOTDIR}/src
TARGETS = test_NetworkGraph
INCDIR = -I. -I${SRCDIR} -I${ROOTDIR}/h -I ${ROOTDIR}/xplat/include
WARNFLAGS= -Wall
LIBMRNET=${ROOTDIR}/lib/i686-pc-linux-gnu/libmrnet.a 
LIBS= ${LIBMRNET} ${ROOTDIR}/xplat/libxplat.a -lpthread -ldl

BISON = bison
FLEX = flex
CXX = g++
CXXFLAGS = -g -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS $(INCDIR) $(WARNFLAGS)

all: ${TARGETS}

test_NetworkGraph: test_NetworkGraph.C parser.tab.o scanner.o ${LIBMRNET}
	${CXX} ${CXXFLAGS} -o $@ parser.tab.o scanner.o test_NetworkGraph.C ${LIBS}
 
parser.tab.C parser.tab.h: $(SRCDIR)/parser.y
	@echo Building `basename $@` ...
	$(BISON) -p mrn -d -y $<
	mv y.tab.c parser.tab.C
	mv y.tab.h parser.tab.h

scanner.C: $(SRCDIR)/scanner.l parser.tab.C
	@echo Building `basename $@` ...
	$(RM)  $@; $(FLEX) -Pmrn -t $< > $@ 

clean:
	$(RM) scanner.C parser.tab.C parser.tab.h *.o

distclean: clean
	$(RM) ${TARGETS}
