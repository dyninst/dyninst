#

# Define any symbols needed to invoke configuration changes in make.config

# XXX get rid of these:

INSTANTIATE_TEMPLATES   = true
DO_INLINE_ALL           = true
#NO_OPT_FLAG = true
#NO_IMPLICIT_TEMPLATES    = true
#AUTO_TEMPLATES 		 = true

# Use /proc support
AIX_PROC	= true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS =		../src/aix.C \
		../src/aixDL.C \
		../src/sol_proc.C \
		../src/inst-power.C \
		../src/inst-aix.C \
		../src/parseStab.C \
		../src/Object-xcoff.C \
		../src/unix.C \
		../src/InstrucIter-power.C \
		../src/FunctionExpansionRecord.C \
		../../common/src/aixv41Kludges.C \
		../../common/src/timing-aix.C \
		../src/writeBackXCOFF.C 
#		../src/addLibraryXCOFF.C


# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Override default of gcc -- need g++ on AIX
LD = $(GXX)
# On AIX all symbols must be resolved at link time, even for a library.
# We need the cplus.demangle function from libiberty.
LDFLAGS += -liberty -Wl,-bbigtoc 

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

# On AIX libraries end in .a for both static and dynamic
TARGET3_SO	= libdyninstAPI.a

#
# Install rule: install withOUT world-readable permissions
# 
install: $(TARGET3_SO)
	-xlC -bnoobjreorder -bexpall -bnoentry -o libdemangle.so.1 -ldemangle -u functionName -u demangle -lc 
	-$(CP) libdemangle.so.1 $(DEST)
	-$(CP) $(TARGET3_SO) $(DEST)
	-chmod 750 $(DEST)/$(TARGET3_SO)
