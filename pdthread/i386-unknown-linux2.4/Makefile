#
# $Log: Makefile,v $
# Revision 1.2  2002/02/26 22:31:01  willb
# Added NO_IMPLICIT_TEMPLATES before including make.config
#
# Revision 1.1  2002/02/26 19:36:41  willb
# initial revision
#
#
# Define any symbols needed to invoke configuration changes in make.config

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

NO_IMPLICIT_TEMPLATES = true

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
