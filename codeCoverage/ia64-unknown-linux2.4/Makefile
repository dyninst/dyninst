# Define any symbols needed to invoke configuration changes in make.config

DEBUG_FLAG = -g -gstabs+
NO_IMPLICIT_TEMPLATES = true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

# Now make any necessary architecture specific changes to variables:

SRCS =          

MYFLAGS          += -I/usr/include/libelf

CXXFLAGS         += $(MYFLAGS)
CFLAGS           += $(MYFLAGS)
TFLAGS           += $(MYFLAGS)

# Tradeoff: adding -g to TFLAGS aids gdb but uses gobs of disk space
#TFLAGS                 += -g

SYSLIBS = -lelf -ldl

AS		= /usr/bin/as

# no support for elf64_XXX() functions
CXXFLAGS	+= -DUSES_ELF32_ONLY
CFLAGS		+= -DUSES_ELF32_ONLY
TFLAGS		+= -DUSES_ELF32_ONLY

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard library template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.program.tmpl
