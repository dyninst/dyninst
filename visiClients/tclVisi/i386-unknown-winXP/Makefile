#
# $Id: Makefile,v 1.1 2003/10/30 00:55:25 rchen Exp $
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
CXXFLAGS	= $(CXXFLAGS) -I.. -D_WINDOWS -MT
TFLAGS		= $(TFLAGS) -I.. -D_WINDOWS -MT
CFLAGS		= $(CFLAGS) -I.. -D_WINDOWS -MT
LDFLAGS		= $(LDFLAGS) -SUBSYSTEM:WINDOWS -NODEFAULTLIB:LIBC.LIB
SYSLIBS		= ws2_32.lib user32.lib iphlpapi.lib

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../nmake.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../../nmake.program.tmpl
