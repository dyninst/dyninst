#
# $Id: Makefile,v 1.1 2003/10/30 00:53:24 rchen Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_FLEX               = true
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

CXXFLAGS		= $(CXXFLAGS) -D_WINDOWS -MT
#CXXFLAGS		= $(CXXFLAGS) -DHIST_DEBUG
SRCS		= $(SRCS) ../../common/src/ntKludges.C
OBJS		= $(OBJS) ntKludges.obj

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

!include ../../nmake.library.tmpl
