BASE            = 
DYNINST_ROOT    = ../../../..
CXX             = g++
CXXFLAGS        = -g -Wall -Dos_linux=24 -Wall -Darch_x86_64 -fno-inline
INCLUDE         =
LDFLAGS         =

PLATFORM        = ppc64_linux
DYNLDFLAGS      = -L/p/paradyn/packages/libelf/lib\
                  -L/p/paradyn/packages/libdwarf/lib\
                  -L$(DYNINST_ROOT)/$(PLATFORM)/lib\
                  -lelf -ldwarf -lcommon -linstructionAPI -lsymtabAPI\
                  -lparseAPI -ldyninstAPI -ldynC_API
DYNINCLUDE      = -I$(DYNINST_ROOT)/dyninst/dyninstAPI/h -I$(DYNINST_ROOT)/include 

#DYNCXXFLAGS     += -Dos_linux=24 -Wall -o -Darch_x86_64

TARG            = myMutatee myMutator
#V               = @  #Verbose (comment to make verbose)

all: $(TARG)

%.o:../%.cpp
	@echo + cc $<
	$(V)$(CXX) -c $(CXXFLAGS) $(INCLUDE) $(DYNINCLUDE) -o $@ $<

myMutatee: testMutatee.o
	@echo + ld $@
	$(V)$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

myMutator: testMutator.o
	@echo + ld $@
	$(V)$(CXX) $(CXXFLAGS) $(LDFLAGS) $(DYNLDFLAGS) -o $@ $^

clean:
	rm -f *.o core.* $(TARG)
