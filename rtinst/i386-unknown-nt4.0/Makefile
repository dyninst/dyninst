#
# $Id: Makefile,v 1.13 2001/02/23 00:01:03 schendel Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

#INCLUDE_CP_PROFILING = true
USES_SHM_SAMPLING = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../nmake.config 

# Now make any necessary architecture specific changes to variables:

# Turn off frame pointer omission optimization for this
# library.  If we allow frame pointer omission, we find that
# call stacks frequently do not contain reference to main 
# or mainCRTStartup, and so are considered invalid call stacks.
!ifndef NO_OPT_FLAG
OPT_FLAG	= $(OPT_FLAG) -Oy-
!endif

LD		= link
LDFLAGS		= -DLL -debug -debugtype:cv -PDB:NONE -base:0x60000000

#TARGET2		= libdyninstCP.a

SRCS		= ../../dyninstAPI_RT/src/RTwinnt.c \
	 	  ../src/RTetc-winnt.c  \
                  ../src/RThwtimer-winnt.c

OBJS		= RTwinnt.obj RTetc-winnt.obj RThwtimer-winnt.obj

CFLAGS		= $(CFLAGS) -Ox -Oy-

# enable debug/trace messages from runtime library
#CFLAGS		= $(CFLAGS) -DDEBUG_PRINT_RT

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../nmake.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../nmake.library.tmpl

{../../dyninstAPI_RT/src/}.c.obj:
	$(CC) -c $(CFLAGS) $<

{../src/}.c.obj:
	$(CC) -c $(CFLAGS) $<

#
# Override target to force ld to happen
#
$(TARGET): $(OBJS)
	@$(MAKE) $(VOUCHER)
	$(LD) $(LDFLAGS) -EXPORT:DYNINSTinit -ENTRY:_DllMainCRTStartup@12 \
	    -out:$(TARGET) $(VOUCHER).obj $(OBJS) $(LIBS) ws2_32.lib winmm.lib

#$(TARGET2): $(OBJS) RTpvmPiggy.o RTcriticalPath.o
#	@$(RM) $(TARGET2)
#	$(LD) $(LDFLAGS) -o $(TARGET2) $(OBJS) RTpvmPiggy.o RTcriticalPath.o $(LIBS)

