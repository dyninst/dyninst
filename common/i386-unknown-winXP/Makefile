#
# $Id: Makefile,v 1.1 2007/01/09 02:00:50 giri Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../nmake.config 
DEFCFLAGS = $(DEFCFLAGS) -D_WINDOWS  -MD # ccw 12 july 2001

# Now make any necessary architecture specific changes to variables:

# we require an uncommon link
LD		= link
LDFLAGS	= -DLL -debug -pdb:none -export:V_libcommon   /NODEFAULTLIB:libc.lib # ccw 20 june 2001


SRCS =		
# ccw 13 july 2001

OBJS = 		
# ccw 13 july 2001

LIBS		= dbghelp.lib advapi32.lib psapi.lib ws2_32.lib

CXXFLAGS 	= $(CXXFLAGS) -DNO_MDL_PRINT -DBPATCH_DLL_BUILD 

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../nmake.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../nmake.library.tmpl

# override target rule to force link to happen
$(TARGET): $(OBJS)
	@$(MAKE) $(VOUCHER)
	$(LD) $(LDFLAGS) -out:$(TARGET) $(VOUCHER).obj $(OBJS) $(LIBS)

install:	$(TARGET)
	$(CP) $(TARGET) $(DEST)
	$(CP) libdyninstAPI.lib $(DEST)

# DO NOT DELETE
