#
# $Id: Makefile,v 1.10 2001/07/09 19:34:51 chadd Exp $
#
#
# Define any symbols needed to invoke configuration changes in make.config

#USES_SHM_SAMPLING = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../nmake.config 

# Now make any necessary architecture specific changes to variables:

LD		= link
LDFLAGS		= -DLL -debug -debugtype:coff -PDB:NONE -base:0x60000000  /NODEFAULTLIB:libc.lib # ccw 20 june 2001

SRCS		= $(SRCS) ../src/RTwinnt.c

OBJS		= RTwinnt.obj

# enable debug/trace messages from library
#CFLAGS		= $(CFLAGS) -DDEBUG_PRINT_RT

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
#	$(LD) $(LDFLAGS) -EXPORT:DYNINSTinit -ENTRY:_DllMainCRTStartup@12 -out:$(TARGET) $(OBJS) $(LIBS) ws2_32.lib winmm.lib


$(TARGET): $(OBJS)
	@$(MAKE) $(VOUCHER)
	$(LD) $(LDFLAGS) -EXPORT:DYNINSTinit -ENTRY:_DllMainCRTStartup@12 \
		-out:$(TARGET) $(VOUCHER).obj $(OBJS) $(LIBS)


#$(TARGET2): $(OBJS) RTpvmPiggy.o RTcriticalPath.o
#	@$(RM) $(TARGET2)
#	$(LD) $(LDFLAGS) -o $(TARGET2) $(OBJS) RTpvmPiggy.o RTcriticalPath.o $(LIBS)

