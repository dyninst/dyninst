#
# $Id: Makefile,v 1.1 2005/03/29 22:58:44 igor Exp $
#

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

TO_CORE		= ../../..
USES_TCLTK	= true
USES_X11        = true
NO_IMPLICIT_TEMPLATES = true

include $(TO_CORE)/make.config 

# Now make any necessary architecture specific changes to variables:
# this two are a temporary kludge; barchart should have a "src" dir

VPATH		+= ..
CXXFLAGS	+= $(TCLFRIENDLYWARNINGS)
CFLAGS		+= $(TCLFRIENDLYWARNINGS)
TFLAGS		+= $(TCLFRIENDLYWARNINGS)


ifdef GCC_2_95
CXXFLAGS	+= -fno-exceptions
TFLAGS		+= -fno-exceptions
endif

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include $(TO_CORE)/make.program.tmpl
