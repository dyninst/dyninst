
USES_TCLTK = true

# include ../Make.common
# DO NOT DELETE

TO_CORE =       ../..
NO_OPT_FLAG=true
NO_IMPLICIT_TEMPLATES=true

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include $(TO_CORE)/make.config

# Now make any necessary architecture specific changes to variables:

#CXXFLAGS += -DUSES_DYNAMIC_INF_HEAP
#CFLAGS += -g -DUSES_DYNAMIC_INF_HEAP

SYSLIBS   += -lnsl 

MUTATEE_CFLAGS += -rdynamic -ldl

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include $(TO_CORE)/make.program.tmpl
