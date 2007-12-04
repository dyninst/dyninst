#

# Define any symbols needed to invoke configuration changes in make.config

# XXX get rid of these:

INSTANTIATE_TEMPLATES   = true
DO_INLINE_ALL           = true
NO_OPT_FLAG = true
#NO_IMPLICIT_TEMPLATES    = true
#AUTO_TEMPLATES 		 = true
OVERRIDE_INSTALL	= true

#DEBUG_FLAG = -g

EXTRA_LIBS = libdemangle.so.1

ifdef COVERAGE
NO_OPT_FLAG		= true
endif

# AIX uses .a for library names...
TARGET	= libsymtabAPI.a

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

SRCS =		../src/Object-xcoff.C \
		../src/Archive.C \
		../src/parseStab.C \
		../src/addrtranslate.C \
		../src/addrtranslate-aix.C

# Add preprocessor and special link information for G++
G_PTHREAD       = -pthread
# Commented out for daily wisc builds
#G_PTHREAD_LD	= -pthread


# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

ifdef IBM_BPATCH_COMPAT
  LDFLAGS += -liberty -bbigtoc -bexpall
  # should be building a static library in this case
  TARGET3_STATIC  = libsymtabAPI.a
  TARGET_INSTALL  = $(TARGET3_STATIC)
  AR = /usr/bin/ar
else
  # On AIX all symbols must be resolved at link time, even for a library.
  # We need the cplus.demangle function from libiberty.
  LDFLAGS += -liberty -Wl,-bbigtoc,-bexpall -lgcc 
  # Override default of gcc -- need g++ on AIX
  LD = $(GXX)
endif


# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

libdemangle.so.1:
	-xlC -bnoobjreorder -bexpall -bnoentry -o libdemangle.so.1 -ldemangle -u functionName -u demangle -u kind -u varName -u text -lc 


#
# Install rule: install withOUT world-readable permissions
# 
install: install_generic libdemangle.so.1
	-$(CP) libdemangle.so.1 $(DEST)
	-chmod 750 $(DEST)/$(TARGET)
