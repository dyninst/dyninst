#
# $Id: Makefile,v 1.3 2003/04/09 18:09:45 pcroth Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_TCLTK	= true
USES_X11	= true
TO_CORE		= ..\..\..
NO_IMPLICIT_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../../nmake.config 

# Now make any necessary architecture specific changes to variables:
CXXFLAGS	= $(CXXFLAGS) -I.. -D_WINDOWS
TFLAGS		= $(TFLAGS) -I.. -D_WINDOWS
CFLAGS		= $(CFLAGS) -I.. -D_WINDOWS
LDFLAGS		= $(LDFLAGS) -SUBSYSTEM:WINDOWS
SYSLIBS		= $(SYSLIBS) ws2_32.lib user32.lib iphlpapi.lib

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../nmake.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../../nmake.program.tmpl
