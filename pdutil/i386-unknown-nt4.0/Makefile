#
# $Id: Makefile,v 1.9 2001/12/06 18:07:34 bernat Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

INSTANTIATE_TEMPLATES   = true
DO_INLINE_ALL           = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.
#NO_OPT_FLAG	= true
!include ../../nmake.config 

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

!include ../nmake.module.tmpl

# Now make any necessary architecture specific changes to variables:

CXXFLAGS		= $(CXXFLAGS) -D_WINDOWS
#CXXFLAGS		= $(CXXFLAGS) -DHIST_DEBUG
SRCS		+= ../../common/src/ntKludges.C
OBJS		+= ntKludges.o

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

!include ../../nmake.library.tmpl
