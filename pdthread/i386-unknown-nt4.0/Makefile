#
# $Id: Makefile,v 1.1 2003/10/28 17:49:46 pcroth Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

#USES_FLEX               = true
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

CXXFLAGS		= $(CXXFLAGS) -D_WINDOWS -Dsnprintf=_snprintf -MT
#LD              = link
#LDFLAGS         = -DLL -debug -pdb:none -export:V_libpdthread
#LIBS            = $(LIBS) libxplat.lib libpdutil.lib kernel32.lib

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

!include ../../nmake.library.tmpl

#$(DEST)\libpdthread.dll: libpdthread.dll
#    $(CP) libpdthread.dll $(DEST)\
#
#$(TARGET): $(OBJS)
#    $(MAKE) $(VOUCHER)
#    $(LD) $(LDFLAGS) -out:$(TARGET) $(VOUCHER).obj $(OBJS) $(LIBS)
#
#install:    $(TARGET)
#    $(CP) $(TARGET) $(DEST)\
#    $(CP) libpdthread.lib $(DEST)\


