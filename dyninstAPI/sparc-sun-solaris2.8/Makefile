#
# $Id: Makefile,v 1.3 2001/10/26 22:45:30 hollings Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

#USES_LIBERTY = true
#USES_SHM_SAMPLING = true
#NO_OPT_FLAG = true
NO_IMPLICIT_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS =		../src/solaris.C \
		../src/solarisDL.C \
		../src/inst-sparc.C \
		../src/inst-sparc-solaris.C \
		../src/inst-sunos.C \
		../src/parseStab.C \
		../src/Object-elf.C \
		../src/tramp-sparc.s \
		../src/unix.C \
		../src/FunctionExpansionRecord.C \
		../src/LocalAlteration.C \
		../src/LocalAlteration-Sparc.C \
		../src/func-reloc.C \
		../src/AddressHandle-sparc.C \
		../../common/src/solarisKludges.C \
		../../common/src/timing-solaris.C


CFLAGS		+= -DNO_MDL_PRINT -DUSES_DYNAMIC_INF_HEAP -g -O0 -I/usr/local/stow/bind-8.2.2.p5/include/
CXXFLAGS	+= -DNO_MDL_PRINT -DUSES_DYNAMIC_INF_HEAP -g -O0 -DUSES_ELF32_ONLY  -I/usr/local/stow/bind-8.2.2.p5/include/
TFLAGS		+= -DUSES_ELF32_ONLY 


# Tradeoff: adding -g to TFLAGS aids gdb but uses gobs of disk space
#TFLAGS 		+= -g

SYSLIBS = -lelf -lsocket -lnsl

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

ifdef USES_LIBDYNINST_SO
$(TARGET3): $(OBJS)
	@$(RM) $(TARGET3)
	@$(MAKE) $(VOUCHER)
	$(CC) -shared -dy -G -z text $(LDFLAGS) -o $(TARGET3) $(VOUCHER).o $(OBJS) $(LIBS)
endif

