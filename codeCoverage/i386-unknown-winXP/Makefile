# Define any symbols needed to invoke configuration changes in make.config

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../nmake.config 
DEFCFLAGS = $(DEFCFLAGS) -MD # ccw 12 july 2001

# Now make any necessary architecture specific changes to variables:

# we require an uncommon link
LD		= link
LDFLAGS	= -debug -pdb:none /NODEFAULTLIB:libc.lib # ccw 20 june 2001


SRCS =		

OBJS = 		

LIBS		= dbghelp.lib advapi32.lib

CXXFLAGS 	= $(CXXFLAGS) -DNO_MDL_PRINT -DBPATCH_DLL_BUILD

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../nmake.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../nmake.program.tmpl

# override target rule to force link to happen

$(TARGET): $(OBJS)
	@$(MAKE) $(VOUCHER)
	$(LD) $(LDFLAGS) -out:$(TARGET) $(VOUCHER).obj $(OBJS) $(LIBS)

