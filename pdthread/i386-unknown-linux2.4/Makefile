#
# $Id: Makefile,v 1.5 2003/07/24 16:44:01 pcroth Exp $
#
# Define any symbols needed to invoke configuration changes in make.config

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

INSTANTIATE_TEMPLATES = true
UNCOMMON_LINK = true

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

CFLAGS	    += -fPIC
CXXFLAGS	+= -fPIC

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

$(TARGET): $(OBJS)
	@$(RM) $@
	@$(MAKE) $(VOUCHER)
	$(CC) -shared $(LDFLAGS) -o $@ $(VOUCHER).o $(OBJS) $(LIBS)

