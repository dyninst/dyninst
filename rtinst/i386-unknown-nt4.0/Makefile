#
# $Log: Makefile,v $
# Revision 1.1  1997/05/23 23:00:37  mjrg
# Windows NT port
#
#
#
# Define any symbols needed to invoke configuration changes in make.config

# none

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

USES_PVM = 1
#USES_SHM_SAMPLING = true

include ../../nmake.config 

# Now make any necessary architecture specific changes to variables:

LD		= link
LDFLAGS		= -DLL -debug -debugtype:coff

#TARGET2		= libdyninstCP.a

DYNOBJS		= DYNINSTstartCode.obj \
		DYNINSTendCode.obj

CFLAGS		= -Z7 $(CFLAGS)

SRCS	     = $(SRCS) ../src/RTfirst.c \
		../src/RTwinnt.c

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
	$(LD) $(LDFLAGS) -EXPORT:DYNINSTinit -ENTRY:_DllMainCRTStartup@12 -out:$(TARGET) $(OBJS) $(LIBS) wsock32.lib winmm.lib

#$(TARGET2): $(OBJS) RTpvmPiggy.o RTcriticalPath.o
#	@rm -f $(TARGET2)
#	$(LD) $(LDFLAGS) -o $(TARGET2) $(OBJS) RTpvmPiggy.o RTcriticalPath.o $(LIBS)

