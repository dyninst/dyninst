#
# $Id: Makefile,v 1.20 2004/03/02 22:45:51 bernat Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_LIBERTY = true
USES_SHM_SAMPLING = true
NO_IMPLICIT_TEMPLATES = true
USES_DWARF_DEBUG = true
# DEBUG_FLAG = -g

#If libhrtime is not installed on your computer and you have no interest
#  in using it, then comment out the following line (with a #), remove
#  the DEPENDS file in this directory, and recompile with make.
#If you have libhrtime, but it can't be found, the libhrtime path can 
#  be adjusted in ../../make.config.  You'll need to remake DEPENDS if
#  this path is adjusted.
#USES_LIBHRTIME = true
#USES_PAPI = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

VPATH        += ../../rtinst/h:../../rtinst/src

SRCS =		../../dyninstAPI/src/linux.C \
		../../dyninstAPI/src/linux-x86.C \
		../../dyninstAPI/src/linuxDL.C \
		../src/init-linux.C \
		../../dyninstAPI/src/linuxMT.C \
		../../dyninstAPI/src/inst-linux.C \
		../../paradynd/src/proc-linux.C \
		../../dyninstAPI/src/inst-x86.C \
		../../dyninstAPI/src/arch-x86.C \
		../../dyninstAPI/src/arch-ia32.C \
		../../dyninstAPI/src/parseDwarf.C \
		../../dyninstAPI/src/parseStab.C \
		../../dyninstAPI/src/Object-elf.C \
		../../dyninstAPI/src/func-reloc.C \
	        ../../dyninstAPI/src/LocalAlteration.C \
	        ../../dyninstAPI/src/LocalAlteration-x86.C \
	        ../../dyninstAPI/src/FunctionExpansionRecord.C \
		../../dyninstAPI/src/unix.C  \
		../../rtinst/src/RThwtimer-linux.c \
		../../dyninstAPI/src/InstrucIter-x86.C \
		../../dyninstAPI/src/addLibraryLinux.C \
		../../dyninstAPI/src/writeBackElf.C \
		../../dyninstAPI/src/saveSharedLibrary.C \
		../../dyninstAPI/src/syscall-linux.C

MYFLAGS		+= -I/usr/include/libelf
MYFLAGS		+= -DNO_MDL_PRINT
MYFLAGS		+= -DUSES_DWARF_DEBUG -I$(LIBDWARFDIR)

SYSLIBS  = -ldl -lelf
#SYSLIBS = -lelf -lsocket -lnsl
LIBS = -L$(LIBDWARFDIR)/lib -ldwarf

AS		= /usr/bin/as

#MYFLAGS		+= -DPDYN_DEBUG
#MYFLAGS		+= -DATTACH_DETACH_DEBUG -DINFERIOR_RPC_DEBUG
#MYFLAGS		+= -DPTRACEDEBUG
#MYFLAGS		+= -DPTRACEDEBUG_EXCESSIVE
#MYFLAGS		+= -DSIGNAL_DEBUG
#MYFLAGS		+= -DINST_TRAP_DEBUG
#MYFLAGS		+= -DLD_DEBUG
#MYFLAGS		+= -DSAMPLEVALUE_DEBUG
#MYFLAGS		+= -DAGGREGATE_DEBUG

CFLAGS		+= $(MYFLAGS)
CXXFLAGS	+= $(MYFLAGS)
TFLAGS		+= $(MYFLAGS)

ifdef USES_LIBHRTIME
LIBS	+= $(HRTIME_LIB)
endif

ifdef USES_PAPI
LIBS	+= $(PAPI_LIB)
endif

ifdef USES_LIBPERFCTR
LIBS    += $(PERFCTR_LIB)
endif

# no support for elf64_XXX() functions
CFLAGS		+= -DUSES_ELF32_ONLY
CXXFLAGS	+= -DUSES_ELF32_ONLY
TFLAGS		+= -DUSES_ELF32_ONLY

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.program.tmpl
