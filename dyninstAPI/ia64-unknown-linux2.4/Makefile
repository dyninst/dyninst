#
# $Id: Makefile,v 1.3 2002/03/16 04:05:27 tlmiller Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

DEBUG_FLAG = -g -gstabs+
NO_IMPLICIT_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

# -O2 or higher breaks Object-elf in interesting ways.
# (Assembler segfaults.)
# (02/27/2002, Todd L Miller; gcc 2.96 (RH 7.1) and gcc 3.0.3 (RH 7.2))
# -O or higher breaks parseStab.c and symtab.C
# (Assembler complains about an unwind directive.)
NO_OPT_FLAG = true
OPT_FLAG =

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS =          ../src/linux.C \
		../src/linuxDL.C \
                ../src/inst-linux.C \
                ../src/inst-ia64.C \
                ../src/arch-ia64.C \
		../src/Object-elf.C \
                ../src/unix.C \
		../src/parseStab.C \
		../src/AddressHandle-ia64.C \
		../src/func-reloc.C \
	        ../src/LocalAlteration.C \
	        ../src/LocalAlteration-ia64.C \
		../src/FunctionExpansionRecord.C \
		../../common/src/linuxKludges.C \
		../../common/src/timing-linux.C


MYFLAGS          += -I/usr/include/libelf

CXXFLAGS         += $(MYFLAGS)
CFLAGS           += $(MYFLAGS)
TFLAGS           += $(MYFLAGS)

# Tradeoff: adding -g to TFLAGS aids gdb but uses gobs of disk space
#TFLAGS           += -g

SYSLIBS = -lelf -ldl

AS		= /usr/bin/as

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl
