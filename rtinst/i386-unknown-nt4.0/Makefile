#
# $Id: Makefile,v 1.3 1998/03/02 00:25:25 wylie Exp $
#
# Define any symbols needed to invoke configuration changes in make.config

# none

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

USES_PVM = true 
#USES_SHM_SAMPLING = true

include ../../nmake.config 

# Now make any necessary architecture specific changes to variables:

LD		= link
LDFLAGS		= -DLL -debug -debugtype:coff

#TARGET2		= libdyninstCP.a

DYNOBJS		= DYNINSTstartCode.obj \
		DYNINSTendCode.obj

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
	@$(MAKE) $(VERSION)
	$(LD) $(LDFLAGS) -EXPORT:DYNINSTinit -ENTRY:_DllMainCRTStartup@12 \
	    -out:$(TARGET) $(VERSION).obj $(OBJS) $(LIBS) wsock32.lib winmm.lib

#$(TARGET2): $(OBJS) RTpvmPiggy.o RTcriticalPath.o
#	@$(RM) $(TARGET2)
#	$(LD) $(LDFLAGS) -o $(TARGET2) $(OBJS) RTpvmPiggy.o RTcriticalPath.o $(LIBS)

