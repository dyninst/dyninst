#
# $Log: Makefile,v $
# Revision 1.1  2005/03/29 22:58:41  igor
# gor
#
# Add ppc64-unknown-linux2.4 as a platform, and make the minimum changes
# needed for kerninst and pdutil to work on linux/ppc64
#
# Revision 1.3  2003/06/05 18:06:02  pcroth
# Removed igen's dependency on the pdutil library.  Igen can now be built
# independently of pdutil.
#
# Revision 1.2  2000/05/31 18:33:22  schendel
# updates the daemon, util library, igen, dyninstAPI, and dyninstAPI test suite
# to use the -fno-impilicit-templates flag instead of the -fexplicit-templates
# flag, which is depracated.
#
# Revision 1.1  2000/03/31 01:58:54  wylie
# Platform migration
#
# Revision 1.1  1997/10/15 19:47:31  tung
# Linux/X86 Port
#
# Revision 1.1  1996/10/18 23:33:19  mjrg
# Solaris/X86 port
#
#
#
# Define any symbols needed to invoke configuration changes in make.config

USES_FLEX = true
NO_IMPLICIT_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS += ../../common/src/linuxKludges.C

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.program.tmpl

#CXX = powerpc64-linux-g++
#LD = powerpc64-linux-ld
