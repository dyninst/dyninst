# $Id: Makefile,v 1.3 1999/05/29 22:30:10 wylie Exp $
#
# Makefile for dyninstAPI test programs on Compaq Alpha
#

# Define any symbols needed to invoke configuration changes in make.config
TO_CORE	= 	../../..
NO_OPT_FLAG=true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:

CXXFLAGS +=
CFLAGS += -g

SYSLIBS   += -g -lmld -liberty

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

ifdef USES_LIBDYNINSTRT_SO
$(TARGET2): $(TARGET2).o
	# $(CC) $(TARGET2).o -o $(TARGET2) ../../../../lib/$(PLATFORM)/libdyninstAPI_RT.so.1
	# $(CC) -taso -g $(TARGET2).o -o $(TARGET2) 
	$(CC) -g $(TARGET2).o -o $(TARGET2) 
else
$(TARGET2): $(TARGET2).o
	$(CC) -g $(TARGET2).o -o $(TARGET2)
endif

libtestA.o:
	$(CC) -g -fpic -c ../src/libtestA.c

libtestB.o:
	$(CC) -fpic -c ../src/libtestB.c

libtestA.so: libtestA.o
	$(CC) -g -shared -o libtestA.so libtestA.o

libtestB.so: libtestB.o
	$(CC) -shared -o libtestB.so libtestB.o

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include $(TO_CORE)/make.program.tmpl
