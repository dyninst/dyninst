# $Log: Makefile,v $
# Revision 1.2  1999/05/13 23:08:26  hollings
# Shared libraries on Alpha.
#
# Revision 1.1  1998/08/25 19:35:42  buck
# Initial commit of DEC Alpha port.
#
# Revision 1.5  1997/09/29 20:32:19  buck
# Merge latest changes from Wisconsin into Maryland repository.
#
# Revision 1.4  1997/08/27 02:07:39  buck
# Merge latest changes from Wisconsin into Maryland repository.
#
# Revision 1.3  1997/08/18 20:47:15  buck
# Merge changes from Wisconsin in to Maryland repository.
#
# Revision 1.2  1997/08/05 19:08:11  hollings
# *** empty log message ***
#
# Revision 1.1.1.2  1997/07/08 20:03:33  buck
# Bring latest changes from Wisconsin over to Maryland repository.
#
# Revision 1.3  1997/07/08 19:15:29  buck
# Added support for the x86 Solaris platform and dynamically linked
# executables to the dyninst API library.
#
# Revision 1.2  1997/06/23 19:16:10  buck
# Added features to the dyninst API library, including an optional "else"
# in a BPatch_ifExpr; the BPatch_setMutationsActive call to temporarily
# disable all snippets; and the replaceFunctionCall and removeFunctionCall
# member functions of BPatch_thread to retarget or NOOP out a function
# call.
#
# Revision 1.1.1.1  1997/04/01 20:25:15  buck
# Update Maryland repository with latest from Wisconsin.
#
# Revision 1.1  1997/03/18 19:45:08  buck
# first commit of dyninst library.  Also includes:
# 	moving templates from paradynd to dyninstAPI
# 	converting showError into a function (in showerror.C)
# 	many ifdefs for BPATCH_LIBRARY in dyinstAPI/src.
#
#
# Makefile for test programs on Sparc Solaris 2.4
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

USES_LIBDYNINSTRT_SO=1

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
