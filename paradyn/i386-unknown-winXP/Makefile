#
# $Id: Makefile,v 1.1 2003/10/30 00:52:30 rchen Exp $
#
# Define any symbols needed to invoke configuration changes in make.config

USES_TCLTK = true
USES_FLEX = true
USES_X11 = true
NO_IMPLICIT_TEMPLATES = true

# Turn off ALL optimization to make debuggering easier.  Remember
#  to remove (and re-compile w/optimization) before committing....
#NO_OPT_FLAG=true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../nmake.config 

# Now make any necessary architecture specific changes to variables:
CFLAGS		= $(CFLAGS) -DSHG_ADD_ON_EVAL -D_WINDOWS
CXXFLAGS	= $(CXXFLAGS) -DSHG_ADD_ON_EVAL -D_WINDOWS -MT
#CXXFLAGS        = $(CXXFLAGS) -DSAMPLEVALUE_DEBUG
TFLAGS		= $(TFLAGS) -DSHG_ADD_ON_EVAL 
LDFLAGS		= $(LDFLAGS) -SUBSYSTEM:WINDOWS -NODEFAULTLIB:LIBC.LIB
SYSLIBS		= $(SYSLIBS) ws2_32.lib user32.lib advapi32.lib iphlpapi.lib

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../nmake.module.tmpl

# Now make any necessary architecture specific changes to variables:

# none to make

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../nmake.program.tmpl
