ifeq ($(DYNINST_ROOT),)
$(error DYNINST_ROOT is not set)
endif

ifeq ($(PLATFORM),)
$(error PLATFORM is not set)
endif

# These should point to where libelf and libdwarf are installed
LIBELF = /p/paradyn/packages/libelf/lib
LIBDWARF = /p/paradyn/packages/libdwarf/lib

DYNINST_INCLUDE = $(DYNINST_ROOT)/${PLATFORM}/include
DYNINST_LIB = $(DYNINST_ROOT)/$(PLATFORM)/lib

CC = gcc
CCFLAGS = -g -Wall -O3

CXX = g++
CXXFLAGS = -g -Wall -O3

EXE = syscall_testing
TEST=helloworld

TARGETS = $(EXE) $(TEST)32 $(TEST)64
all:$(TARGETS)

TOOLS=${HOME}/paradyn/Dyninst/tools

DYNINST_LIBS = -L$(DYNINST_LIB) \
			   -L$(LIBELF) -L$(LIBDWARF) \
			   -lcommon \
			   -ldwarf -lelf \
			   -lpcontrol \

SRCS=syscall_testing.C

HEADERS=$(SRCS:.C=.h)
OBJS=$(SRCS:.C=.o)

helloworld: helloworld.c
	$(CC) -o helloworld helloworld.c

helloworld32: helloworld.c
	$(CC) -m32 -o helloworld32 helloworld.c

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(DFLAGS) -I$(DYNINST_INCLUDE) $(DYNINST_LIBS) $(HELPER_LIBS) -o $@ $^ 

%.o: %.C 
	$(CXX) $(CXXFLAGS) $(DFLAGS) -I$(DYNINST_INCLUDE) -I$(LIBELF) -I$(LIBDWARF) -c $*.C

clean: 
	rm -f *.o $(TARGETS) core.* tmp

