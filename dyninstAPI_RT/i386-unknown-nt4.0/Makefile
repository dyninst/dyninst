#
# $Log: Makefile,v $
# Revision 1.1  1997/08/18 01:34:45  buck
# Ported the Dyninst API to Windows NT.
#
# Revision 1.1.1.1  1997/06/11 17:40:12  buck
# Update Maryland repository with latest changes from Wisconsin.
#
# Revision 1.1  1997/05/23 23:00:37  mjrg
# Windows NT port
#
#
#
# Define any symbols needed to invoke configuration changes in make.config

# none

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

#USES_PVM = 1
#USES_SHM_SAMPLING = true

include ../../nmake.config 

# Now make any necessary architecture specific changes to variables:

LD		= link
LDFLAGS		= -DLL -debug -debugtype:coff

CFLAGS		= -Z7 $(CFLAGS)

SRCS		= $(SRCS) ../src/RTwinnt.c

OBJS		= RTwinnt.obj

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../nmake.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../nmake.library.tmpl

{../src/}.c.obj:
	$(CC) -c $(CFLAGS) $<

#
# Override target to force ld to happen
#
#$(TARGET): $(OBJS)
#	$(LD) $(LDFLAGS) -EXPORT:DYNINSTinit -ENTRY:_DllMainCRTStartup@12 -out:$(TARGET) $(OBJS) $(LIBS) wsock32.lib winmm.lib


$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -EXPORT:DYNINSTinit -ENTRY:_DllMainCRTStartup@12 -out:$(TARGET) $(OBJS) $(LIBS)


#$(TARGET2): $(OBJS) RTpvmPiggy.o RTcriticalPath.o
#	@rm -f $(TARGET2)
#	$(LD) $(LDFLAGS) -o $(TARGET2) $(OBJS) RTpvmPiggy.o RTcriticalPath.o $(LIBS)

