# test application

# Set this to your Dyninst installation directory
DYNINST_ROOT = /p/paradyn/development/bernat/builds

# Set this to the location of additional libraries (libelf/libdwarf)
EXTRA_LIBS	= -L/p/paradyn/packages/libelf/lib -L/p/paradyn/packages/libdwarf/lib

CXX		=	g++
CC		=	gcc
LDFLAGS		=	$(EXTRA_LIBS) -L$(DYNINST_ROOT)/$(PLATFORM)/lib -liberty -lelf -ldwarf -ldyninstAPI -lsymtabAPI
INCLUDE		=	-I$(DYNINST_ROOT)/include
CPPFLAGS	=	-O2 -g

all: wrapper lib_wrapper.so lib_wrapper_cxx.so lib_wrapper.o test test_cxx test_static

wrapper: ../src/wrapper.C
	$(CXX)	$(CPPFLAGS) $(INCLUDE) $(LDFLAGS) ../src/wrapper.C -o $@

test: ../src/test.c
	$(CC) -o $@ ../src/test.c
test_static: ../src/test.c
	$(CC) -static -o $@ ../src/test.c
test_cxx: ../src/test.c
	$(CXX) -o $@ ../src/test.c

lib_wrapper.so: ../src/lib_wrapper.c
	$(CC) -shared ../src/lib_wrapper.c -o lib_wrapper.so -fPIC
lib_wrapper_cxx.so: ../src/lib_wrapper.c
	$(CXX) -shared ../src/lib_wrapper.c -o lib_wrapper_cxx.so -fPIC
lib_wrapper.o: ../src/lib_wrapper.c
	$(CC) -c ../src/lib_wrapper.c -fPIC
clean:
	rm -f wrapper test test_static test_cxx lib_wrapper.so lib_wrapper_cxx.so lib_wrapper.o
