#
# $Id: Makefile,v 1.7 1998/07/23 16:13:33 wylie Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

#INCLUDE_CP_PROFILING = true
#USES_SHM_SAMPLING = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../nmake.config 

# Now make any necessary architecture specific changes to variables:

LD		= link
LDFLAGS		= -DLL -debug -debugtype:coff -PDB:NONE -base:0x60000000

#TARGET2		= libdyninstCP.a

SRCS	     = $(SRCS) ../src/RTfirst.c ../src/RTwinnt.c

OBJS		= RTfirst.obj RTwinnt.obj

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../nmake.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../nmake.library.tmpl

{../src/}.c.obj:
	$(CC) -c $(CFLAGS) $<

#
# Override target to force ld to happen
#
$(TARGET): $(OBJS)
	@$(MAKE) $(VOUCHER)
	$(LD) $(LDFLAGS) -EXPORT:DYNINSTinit -ENTRY:_DllMainCRTStartup@12 \
	    -out:$(TARGET) $(VOUCHER).obj $(OBJS) $(LIBS) wsock32.lib winmm.lib

#$(TARGET2): $(OBJS) RTpvmPiggy.o RTcriticalPath.o
#	@$(RM) $(TARGET2)
#	$(LD) $(LDFLAGS) -o $(TARGET2) $(OBJS) RTpvmPiggy.o RTcriticalPath.o $(LIBS)

