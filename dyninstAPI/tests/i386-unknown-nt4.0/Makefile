#
# Makefile for dyninst test programs on Windows NT.
#

TO_CORE=../../..

# Define any symbols needed to invoke configuration changes in nmake.config

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

!include ../../../nmake.config

# Now make any necessary architecture specific changes to variables:

CFLAGS = /Z7

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

!include ../nmake.module.tmpl

$(TARGET2): $(TARGET2:.exe=.obj)
	link -out:$(TARGET2) -debug -debugtype:coff $(TARGET2:.exe=.obj)

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

!include ../../../nmake.program.tmpl
