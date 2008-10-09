MRNET_ROOT     = ?PLEASE SET TO YOUR MRNET DISTRIBUTION DIRECTORY?
MRNET_PLATFORM = ?PLEASE SET TO YOUR PLATFORM?
CXX = g++

# Only edit below this line if you really need to

LIBDIR   = $(MRNET_ROOT)/lib/${MRNET_PLATFORM}
LIBMRNET = $(LIBDIR)/libmrnet.a
LIBXPLAT = $(MRNET_ROOT)/xplat/libxplat.a
LIBS     = $(LIBMRNET) $(LIBXPLAT) -lfl -lm -lpthread -ldl

INCDIR   = -I$(MRNET_ROOT)/include -I$(MRNET_ROOT)/xplat/include -I$(MRNET_ROOT)/src

CXXFLAGS = -g $(INCDIR)
SOFLAGS = -fPIC -shared -rdynamic
LDFLAGS = -rdynamic

BA_TARGETS = BackendAttach_FE BackendAttach_BE
IA_TARGETS = IntegerAddition_FE IntegerAddition_BE IntegerAdditionFilter.so
TARGETS = $(BA_TARGETS) $(IA_TARGETS)

.PHONY: all backend-attach integer-add clean

all: $(TARGETS)

backend-attach: $(BA_TARGETS) 

integer-add: $(IA_TARGETS)

BackendAttach_FE: NoBackEndInstantiation/FE.C
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ NoBackEndInstantiation/FE.C $(LIBS)

BackendAttach_BE: NoBackEndInstantiation/BE.C
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ NoBackEndInstantiation/BE.C $(LIBS)

IntegerAddition_FE: IntegerAddition/IntegerAddition_FE.C
	$(CXX) $(CXXFLAGS) -IIntegerAddition $(LDFLAGS) -o $@ IntegerAddition/IntegerAddition_FE.C $(LIBS)

IntegerAddition_BE: IntegerAddition/IntegerAddition_BE.C
	$(CXX) $(CXXFLAGS) -IIntegerAddition $(LDFLAGS) -o $@ IntegerAddition/IntegerAddition_BE.C $(LIBS)

IntegerAdditionFilter.so: IntegerAddition/IntegerAdditionFilter.C
	$(CXX) $(CXXFLAGS) $(SOFLAGS) -o $@ IntegerAddition/IntegerAdditionFilter.C

clean:
	/bin/rm -f *.o *~ $(TARGETS)
