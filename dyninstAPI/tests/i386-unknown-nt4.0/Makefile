#
# Makefile for dyninst test programs on Windows NT.
#
# $Id: Makefile,v 1.17 2005/02/09 03:27:48 jaw Exp $
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
#Support for GNU compilers is being dropped since it seems to be rarely used.
#M_GCC = gcc
#M_GXX = g++
NATIVE_CC = VC
NATIVE_CXX = VC++

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

!include ../nmake.module.tmpl

# TARGET2 is for mutatees
!ifdef TARGET2

!if ("$(MUTATEE_CC)"=="$(NATIVE_CC)")
COMPILER="Microsoft_(R)_32-bit_C/C++_Optimizing_Compiler_Version_12.00.8168_for_80x86"
MUTATEE_CFLAGS = -nologo -Z7 -W3 $(UNIFIED_DEF) -GR -DCOMPILER=\"$(COMPILER)\" -MD
LDFLAGS = -nologo -debug -pdb:none
#LDFLAGS = -nologo -debug 
!ifdef MUTATEE_ASM
MUTATEE_OBJ=$(MUTATEE_ASM:.asm=.obj)
MUTATEE_LST=$(MUTATEE_ASM:.asm=.lst)
$(MUTATEE_OBJ): ../src/$(MUTATEE_ASM)
	$(NASM) -o $(MUTATEE_OBJ) -l $(MUTATEE_LST) -fwin32 -dPLATFORM=$(PLATFORM) ../src/$(MUTATEE_ASM)
$(TARGET2): ../src/$(TARGET2:_VC.exe=.c) $(MUTATEE_OBJ) 
	cl -TC $(MUTATEE_CFLAGS) ../src/$(TARGET2:_VC.exe=.c) \
		-o $(TARGET2) -link $(LDFLAGS) $(MUTATEE_OBJ)
!else
$(TARGET2): ../src/$(TARGET2:_VC.exe=.c) $(MUTATEE_SRC)
#	cl -TC $(MUTATEE_CFLAGS) -c ../src/$(TARGET2:_VC.exe=.c)
#	$(LINK) -out:$(TARGET2) $(LDFLAGS) $(TARGET2:_VC.exe=.obj)
	cl -TC $(MUTATEE_CFLAGS) ../src/$(TARGET2:_VC.exe=.c) $(MUTATEE_SRC) \
		-o $(TARGET2) -link $(LDFLAGS)
!endif
!endif

!if ("$(MUTATEE_CC)"=="$(NATIVE_CXX)")
COMPILER="Microsoft_(R)_32-bit_C/C++_Optimizing_Compiler_Version_12.00.8168_for_80x86"
MUTATEE_CFLAGS = -nologo -Z7 -W3 $(UNIFIED_DEF) -GR -GX -DCOMPILER=\"$(COMPILER)\" -MD
LDFLAGS = -nologo -debug -pdb:none
#LDFLAGS = -nologo -debug 
$(TARGET2): ../src/$(TARGET2:_VC++.exe=.c)
#	cl -TP $(MUTATEE_CFLAGS) -c ../src/$(TARGET2:_VC++.exe=.c)
#	$(LINK) -out:$(TARGET2) $(LDFLAGS) $(TARGET2:_VC++.exe=.obj)
	cl -TP $(MUTATEE_CFLAGS) ../src/$(TARGET2:_VC++.exe=.c) $(MUTATEE_SRC) \
		-o $(TARGET2) -link $(LDFLAGS)
!endif

!if ("$(MUTATEE_CC)"=="$(M_GCC)")
COMPILER="gcc_version_egcs-2.91.57_19980901_(egcs-1.1 release)"
MUTATEE_CFLAGS = -g $(UNIFIED_DEF) -Wall -DCOMPILER='$(COMPILER)'
$(TARGET2): ../src/$(TARGET2:_gcc.exe=.c)
	$(M_GCC) -o $(TARGET2) $(MUTATEE_CFLAGS) ../src/$(TARGET2:_gcc.exe=.c)
!endif

!if ("$(MUTATEE_CC)"=="$(M_GXX)")
COMPILER="gcc_version_egcs-2.91.57_19980901_(egcs-1.1 release)"
MUTATEE_CFLAGS = -g $(UNIFIED_DEF) -Wall -DCOMPILER='$(COMPILER)'
$(TARGET2): ../src/$(TARGET2:_g++.exe=.c)
	$(M_GXX) -o $(TARGET2) $(MUTATEE_CFLAGS) ../src/$(TARGET2:_g++.exe=.c)
!endif

!endif TARGET2

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

!include ../../../nmake.program.tmpl
