#
# $Id: Makefile,v 1.1 2000/03/31 01:58:27 wylie Exp $
#

# Define any symbols needed to invoke configuration changes in make.config
TO_CORE	= 	../../..
NO_OPT_FLAG=true

DEBUG_FLAG = -gstabs+

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:

CXXFLAGS += 
CFLAGS +=

SYSLIBS   += -lelf -lnsl

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

MUTATEE_SRC = ../src/$(TARGET2:_$(MUTATEE_CC)=.c)

ifdef USES_LIBDYNINSTRT_SO
$(TARGET2): $(MUTATEE_SRC)
	$(MUTATEE_CC) $(MODCFLAGS) -g $(MUTATEE_SRC) -rdynamic -ldl -o $(TARGET2)
else
DYNINSTAPI_RT_LIB=$(TO_CORE)/$(LIBRARY_DEST)/libdyninstAPI_RT.o

$(TARGET2): $(MUTATEE_SRC)
	$(MUTATEE_CC) $(DYNINSTAPI_RT_LIB) -g $(MUTATEE_SRC) -ldl -o $(TARGET2)
endif

libtestA.o:
	$(CC) -g -fpic -c ../src/libtestA.c

libtestB.o:
	$(CC) -g -fpic -c ../src/libtestB.c

libtestA.so: libtestA.o
	$(CC) -shared -o libtestA.so libtestA.o

libtestB.so: libtestB.o
	$(CC) -shared -o libtestB.so libtestB.o

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include $(TO_CORE)/make.program.tmpl
