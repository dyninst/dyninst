#
# $Id: Makefile,v 1.5 2005/10/14 16:37:40 legendre Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

# none

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

USES_LIBELF = true

include ../../make.config 

# Now make any necessary architecture specific changes to variables:
LD		= $(GCC)
LDFLAGS         += -shared -nostdlib

# Automatic init routine
LDFLAGS		+= -Wl,-init,_start 
CFLAGS		+= -Wall

# if we want to produce a shared object...
CC              = $(GCC)
LD              = $(GCC)
CFLAGS          += -I. -I../.. -I$(LIBELF_INC) -I/usr/include/libelf -fPIC

LIBS 		+= -ldl -L$(LIBELF_LIB)
ifdef LIBELFSTATIC
LIBS    += -Wl,-static -lelf -Wl,-call_shared
else
LIBS    += -lelf
endif

ASFLAGS         += 

SRCS		+= ../src/RTposix.c \
		   ../src/RTlinux.c \
		   ../src/RTheap.c \
		   ../src/RTheap-linux.c \
		   ../src/RTmutatedBinary.c \
		   ../src/RTmutatedBinary_ELF.c \
			../src/RTthread-x86-64.c \
			../src/RTthread.c \
			../src/RTthread-index.c

#CFLAGS += -DDEBUG_PRINT_RT    # enable debug/trace messages from library

ifdef DETACH_ON_THE_FLY
CFLAGS += -DDETACH_ON_THE_FLY
SRCS += ../src/RTsigill-x86.S
endif

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# 32-bit runtime library
ASM_SRCS_32  = ../src/RTfake-ret-x86.S

TARGET_32_SO = libdyninstAPI_RT_m32.so.1
XTARGET_32   = $(basename $(basename $(TARGET_32_SO)))
VOUCHER_32   = V_$(XTARGET_32)
OBJS_32      = $(patsubst %.c, %_m32.o, $(filter %.c,$(notdir $(SRCS))))
ASM_OBJS_32  = $(patsubst %.S, %_m32.o, $(filter %.S,$(notdir $(ASM_SRCS_32))))

CFLAGS_32    = -m32 -DMUTATEE_32 $(CFLAGS)
LDFLAGS_32   = -m32 $(LDFLAGS)
LIBS_32      = -L/usr/lib $(LIBS)

$(OBJS_32): %_m32.o: ../src/%.c
	$(CC) $(CFLAGS_32) -c $< -o $@
$(ASM_OBJS_32): %_m32.o: ../src/%.S
	$(CC) $(CFLAGS_32) -c $< -o $@
$(VOUCHER_32):
	@echo Updating build voucher information: $(VOUCHER_32)
	$(BUILDSTAMP) $(BUILD_ID) $(XTARGET)
	$(MODCC) -m32 -c $(VOUCHER).c -o $(VOUCHER_32).o
$(TARGET_32_SO): $(OBJS_32) $(ASM_OBJS_32)
	@$(RM) $(TARGET_32_SO)
	@$(MAKE) $(VOUCHER_32)
	$(LD) $(LDFLAGS_32) -o $(TARGET_32_SO) $(VOUCHER_32).o $(OBJS_32) $(ASM_OBJS_32) $(LIBS_32)

$(DEST)/$(TARGET_32_SO): $(TARGET_32_SO)
	-$(CP) $(TARGET_32_SO) $(DEST)
install: $(DEST)/$(TARGET_32_SO)
all: $(TARGET_32_SO)

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

# This is just a dirty hack to make the build process complete
# smoothly.  It should be removed when we can correctly build
# a 64-bit runtime library.
#$(TARGET3_SO):
#	touch $(TARGET3_SO)
