#
# $Log: Makefile,v $
# Revision 1.1  1997/05/23 22:59:12  mjrg
# Windows NT port
#
# Revision 1.8  1995/04/07  17:20:17  jcargill
# Enable INLINING since we're not using external templates
#
# Revision 1.7  1995/04/06  22:19:52  jcargill
# Moved setting of INSTANTIATE_TEMPLATES down into arch-specific Makefiles,
# since it must be set before make.config is read.
#
# Revision 1.6  1995/02/16  09:24:27  markc
# Added template support
# Template support.
# Removed compiler warnings
# Template support.
# Removed compiler warnings
# Template support.
# Removed compiler warnings
# Template support.
# Removed compiler warnings
#
# Revision 1.23  1995/01/30  18:11:58  jcargill
# Major build system reorganization
#
#
# Define any symbols needed to invoke configuration changes in make.config

INSTANTIATE_TEMPLATES   = true
DO_INLINE_ALL           = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

!include ../../nmake.config 

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

!include ../nmake.module.tmpl

# Now make any necessary architecture specific changes to variables:

#CXXFLAGS		= $(CXXFLAGS)

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

!include ../../nmake.library.tmpl
