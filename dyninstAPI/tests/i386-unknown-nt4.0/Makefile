#
# Makefile for dyninst test programs on Windows NT.
#
# $Id: Makefile,v 1.10 2001/07/12 20:25:48 chadd Exp $
#

# Define any symbols needed to invoke configuration changes in nmake.config

TO_CORE=..\..\..
NO_OPT_FLAG=true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

!include ../../../nmake.config
DEFCFLAGS = $(DEFCFLAGS) -MD # ccw 12 july 2001
LDFLAGS = $(LDFLAGS) /NODEFAULTLIB:LIBC.lib  # ccw 12 july 2001
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
COMPILER="Microsoft_(R)_32-bit_C/C++_Optimizing_Compiler_Version_12.00.8168_for_80x86"
MUTATEE_CFLAGS = -nologo -Z7 -W3 $(ARCH_DEF) -GR -DCOMPILER=\"$(COMPILER)\"
LDFLAGS = -nologo -debug -pdb:none
$(TARGET2): ../src/$(TARGET2:_VC.exe=.c)
#	cl -TC $(MUTATEE_CFLAGS) -c ../src/$(TARGET2:_VC.exe=.c)
#	$(LINK) -out:$(TARGET2) $(LDFLAGS) $(TARGET2:_VC.exe=.obj)
	cl -TC $(MUTATEE_CFLAGS) ../src/$(TARGET2:_VC.exe=.c) \
		-o $(TARGET2) -link $(LDFLAGS)
!endif

!if ("$(MUTATEE_CC)"=="$(NATIVE_CXX)")
COMPILER="Microsoft_(R)_32-bit_C/C++_Optimizing_Compiler_Version_12.00.8168_for_80x86"
MUTATEE_CFLAGS = -nologo -Z7 -W3 $(ARCH_DEF) -GR -GX -DCOMPILER=\"$(COMPILER)\"
LDFLAGS = -nologo -debug -pdb:none
$(TARGET2): ../src/$(TARGET2:_VC++.exe=.c)
#	cl -TP $(MUTATEE_CFLAGS) -c ../src/$(TARGET2:_VC++.exe=.c)
#	$(LINK) -out:$(TARGET2) $(LDFLAGS) $(TARGET2:_VC++.exe=.obj)
	cl -TP $(MUTATEE_CFLAGS) ../src/$(TARGET2:_VC++.exe=.c) \
		-o $(TARGET2) -link $(LDFLAGS)
!endif

!if ("$(MUTATEE_CC)"=="$(GNU_CC)")
COMPILER="gcc_version_egcs-2.91.57_19980901_(egcs-1.1 release)"
MUTATEE_CFLAGS = -g $(ARCH_DEF) -Wall -DCOMPILER='$(COMPILER)'
$(TARGET2): ../src/$(TARGET2:_gcc.exe=.c)
	$(GNU_CC) -o $(TARGET2) $(MUTATEE_CFLAGS) ../src/$(TARGET2:_gcc.exe=.c)
!endif

!if ("$(MUTATEE_CC)"=="$(GNU_CXX)")
COMPILER="gcc_version_egcs-2.91.57_19980901_(egcs-1.1 release)"
MUTATEE_CFLAGS = -g $(ARCH_DEF) -Wall -DCOMPILER='$(COMPILER)'
$(TARGET2): ../src/$(TARGET2:_g++.exe=.c)
	$(GNU_CXX) -o $(TARGET2) $(MUTATEE_CFLAGS) ../src/$(TARGET2:_g++.exe=.c)
!endif

!endif TARGET2

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

!include ../../../nmake.program.tmpl
