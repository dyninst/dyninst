#
# $Id: Makefile,v 1.10 2007/12/04 18:05:37 legendre Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

#USES_LIBERTY = true
#NO_OPT_FLAG = true
DEBUG_FLAG = -g
NO_IMPLICIT_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS =		../src/Object-elf.C \
                ../src/parseDwarf.C \
		../src/parseStab.C \
		../src/emitElf.C \
		../src/emitElf-64.C \
		../src/addrtranslate.C \
		../src/addrtranslate-sysv.C \
		../src/addrtranslate-solaris.C

CFLAGS		+= -DNO_MDL_PRINT -I/usr/local/stow/bind-8.2.2.p5/include/
#CFLAGS		+= -DNO_MDL_PRINT -O0 -I/usr/local/stow/bind-8.2.2.p5/include/ 
CXXFLAGS	+= -DNO_MDL_PRINT -DUSES_ELF32_ONLY  -DUSES_DWARF_DEBUG -I/usr/local/stow/bind-8.2.2.p5/include/ -DUSES_DWARF_DEBUG -I$(LIBDWARF_INC)
#CXXFLAGS	+= -DNO_MDL_PRINT -O0 -DUSES_ELF32_ONLY  -I/usr/local/stow/bind-8.2.2.p5/include/
TFLAGS		+= -DUSES_ELF32_ONLY 

LDFLAGS += -R$(LIBDWARF_LIB):$(LIBELF_LIB) -L$(LIBELF_LIB) -lelf -L$(LIBDWARF_LIB) -ldwarf
SYSLIBS = -lsocket -lnsl -ldl -lrt

# Add preprocessor and special link information for G++
G_PTHREAD       = -pthreads
G_PTHREAD_LD	= -pthreads

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

#ifdef USES_LIBDYNINST_SO
#$(TARGET3): $(OBJS)
#	@$(RM) $(TARGET3)
#	@$(MAKE) $(VOUCHER)
#	$(CC) -shared -dy -G -z text $(LDFLAGS) -o $(TARGET3) $(VOUCHER).o $(OBJS) $(LIBS)
#endif

