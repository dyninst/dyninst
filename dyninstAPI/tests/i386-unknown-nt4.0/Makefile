#
# Makefile for dyninst test programs on Windows NT.
#
# $Id: Makefile,v 1.6 2000/04/24 02:37:32 wylie Exp $
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
GNU_CXX = g++
NATIVE_CC = VC
NATIVE_CXX = VC++

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

!include ../nmake.module.tmpl

# TARGET2 is for mutatees
!ifdef TARGET2

!if ("$(MUTATEE_CC)"=="$(NATIVE_CC)")
MUTATEE_CFLAGS = -nologo -Z7 -W3 $(ARCH_DEF) -GR
LDFLAGS = -nologo -debug -pdb:none
$(TARGET2):
	cl -TC $(MUTATEE_CFLAGS) -c ../src/$(TARGET2:_VC.exe=.c)
	$(LINK) -out:$(TARGET2) $(LDFLAGS) $(TARGET2:_VC.exe=.obj)
!endif

!if ("$(MUTATEE_CC)"=="$(NATIVE_CXX)")
MUTATEE_CFLAGS = -nologo -Z7 -W3 $(ARCH_DEF) -GR -GX
LDFLAGS = -nologo -debug -pdb:none
$(TARGET2):
	cl -TP $(MUTATEE_CFLAGS) -c ../src/$(TARGET2:_VC++.exe=.c)
	$(LINK) -out:$(TARGET2) $(LDFLAGS) $(TARGET2:_VC++.exe=.obj)
!endif

!if ("$(MUTATEE_CC)"=="$(GNU_CC)")
MUTATEE_CFLAGS = -g $(ARCH_DEF) -Wall
$(TARGET2): ../src/$(TARGET2:_gcc.exe=.c)
	$(GNU_CC) -o $(TARGET2) $(MUTATEE_CFLAGS) -c ../src/$(TARGET2:_gcc.exe=.c)
!endif

!if ("$(MUTATEE_CC)"=="$(GNU_CXX)")
MUTATEE_CFLAGS = -g $(ARCH_DEF) -Wall
$(TARGET2): ../src/$(TARGET2:_g++.exe=.c)
	$(GNU_CXX) -o $(TARGET2) $(MUTATEE_CFLAGS) -c ../src/$(TARGET2:_g++.exe=.c)
!endif

!endif TARGET2

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

!include ../../../nmake.program.tmpl
