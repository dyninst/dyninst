#
# $Id: Makefile,v 1.1 2000/03/31 16:56:18 wylie Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

TO_CORE		= ../../..
USES_TCLTK	= true
USES_X11        = true
NO_IMPLICIT_TEMPLATES = true

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

SRCS += ../src/templates-X.C

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include $(TO_CORE)/make.program.tmpl

templates-X.o: ../src/templates-X.C
	$(CXX) $(TFLAGS) -c ../src/templates-X.C
