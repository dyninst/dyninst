#
# Makefile for dyninst test programs on Windows NT.
#
# $Id: Makefile,v 1.5 1999/11/07 21:50:01 wylie Exp $
#

# Define any symbols needed to invoke configuration changes in nmake.config

TO_CORE=..\..\..
NO_OPT_FLAG=true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

!include ../../../nmake.config

# Now make any necessary architecture specific changes to variables:

CFLAGS = /Z7
GNU_CC = gcc
NATIVE_CC = cl

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

!include ../nmake.module.tmpl

# TARGET2 is for mutatees
!ifdef TARGET2
!if ("$(MUTATEE_CC)"=="$(NATIVE_CC)")
$(TARGET2): $(TARGET2:_cl.exe=.obj)
#	$(LINK) -out:$(TARGET2) $(LDFLAGS) -debug -debugtype:coff
	$(LINK) -out:$(TARGET2) $(LDFLAGS) -debug -pdb:none \
                $(TARGET2:_cl.exe=.obj)
!endif
!if ("$(MUTATEE_CC)"=="$(GNU_CC)")
$(TARGET2): ../src/$(TARGET2:_gcc.exe=.c)
	$(GNU_CC) -o $(TARGET2) -g -c ../src/$(TARGET2:_gcc.exe=.c)
!endif
!endif

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

!include ../../../nmake.program.tmpl
