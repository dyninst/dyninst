#
# Makefile for dyninst test programs on Windows NT.
#
# $Id: Makefile,v 1.4 1999/11/06 21:34:17 wylie Exp $
#

# Define any symbols needed to invoke configuration changes in nmake.config

TO_CORE=..\..\..
NO_OPT_FLAG=true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

!include ../../../nmake.config

# Now make any necessary architecture specific changes to variables:

CFLAGS = /Z7

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

!include ../nmake.module.tmpl

!ifdef TARGET2
$(TARGET2): $(TARGET2:.exe=.obj)
	$(LINK) -out:$(TARGET2) $(LDFLAGS) -debug -debugtype:coff $(TARGET2:.exe=.obj)
!endif

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

!include ../../../nmake.program.tmpl
