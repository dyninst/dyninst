#
# $Id: Makefile,v 1.1 2003/09/30 14:20:45 jodom Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_FLEX = true
NO_IMPLICIT_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS += ../../common/src/solarisKludges.C

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.program.tmpl

