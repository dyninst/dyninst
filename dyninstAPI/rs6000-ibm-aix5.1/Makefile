#

# Define any symbols needed to invoke configuration changes in make.config

# XXX get rid of these:

INSTANTIATE_TEMPLATES   = true
DO_INLINE_ALL           = true
#NO_OPT_FLAG = true
#NO_IMPLICIT_TEMPLATES    = true
#AUTO_TEMPLATES 		 = true


ifdef COVERAGE
NO_OPT_FLAG		= true
endif

# Use /proc support
AIX_PROC	= true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 
ifdef IBM_BPATCH_COMPAT
# use XLC auto template mechanism
AUTO_TEMPLATES = true
endif

ifdef COVERAGE
CXXFLAGS += -O0
endif

# Now make any necessary architecture specific changes to variables:

SRCS =		../src/aix.C \
		../src/aixMT.C \
		../src/aixDL.C \
		../src/dynamiclinking.C \
		../src/sol_proc.C \
		../src/inst-power.C \
		../src/inst-aix.C \
		../src/parseStab.C \
		../src/Object-xcoff.C \
		../src/unix.C \
		../src/InstrucIter-power.C \
		../../common/src/aixv41Kludges.C \
		../../common/src/timing-aix.C \
		../src/writeBackXCOFF.C  \
		../src/syscall-solproc.C
#		../src/addLibraryXCOFF.C


# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

ifdef IBM_BPATCH_COMPAT
  LDFLAGS += -liberty -bbigtoc 
  # should be building a static library in this case
  TARGET3_STATIC  = libdyninstAPI.a
  TARGET_INSTALL  = $(TARGET3_STATIC)
  AR = /usr/bin/ar
else
  # On AIX all symbols must be resolved at link time, even for a library.
  # We need the cplus.demangle function from libiberty.
  LDFLAGS += -liberty -Wl,-bbigtoc -lgcc
  # Override default of gcc -- need g++ on AIX
  LD = $(GXX)
endif


# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

# On AIX libraries end in .a for both static and dynamic
ifndef STATIC_DYNINST_LIB
  TARGET3_SO	= libdyninstAPI.a
  TARGET_INSTALL  = $(TARGET3_SO)
endif


#
# Install rule: install withOUT world-readable permissions
# 
install: $(TARGET_INSTALL)
	-xlC -bnoobjreorder -bexpall -bnoentry -o libdemangle.so.1 -ldemangle -u functionName -u demangle -u kind -u varName -u text -lc 
	-$(CP) libdemangle.so.1 $(DEST)
	-$(CP) $(TARGET_INSTALL) $(DEST)
	-chmod 750 $(DEST)/$(TARGET_INSTALL)
