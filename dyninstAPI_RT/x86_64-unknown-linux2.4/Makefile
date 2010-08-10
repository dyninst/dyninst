#
# $Id: Makefile,v 1.13 2008/04/11 23:30:47 legendre Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

# none

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include ../../make.config 

ifndef STATIC_TARGET
    STATIC_TARGET = libdyninstAPI_RT.a
endif

# Now make any necessary architecture specific changes to variables:
LD		= $(GCC)
LDFLAGS         += -shared
LDFLAGS		+= -m64
CFLAGS		+= -Wall -m64 -DMUTATEE64

# if we want to produce a shared object...
CC              = $(GCC)
LD              = $(GCC)
STATIC_CFLAGS   := $(CFLAGS) -DDYNINST_RT_STATIC_LIB -I. -I../..
CFLAGS          += -I. -I../.. -fPIC

LIBS 		+= -ldl

ASFLAGS         += 

SRCS		+= ../src/RTposix.c \
		   ../src/RTlinux.c \
		   ../src/RTheap.c \
		   ../src/RTheap-linux.c \
			../src/RTthread-x86-64.c \
			../src/RTsignal-x86_64.S \
			../src/RTthread.c \
			../src/RTthread-index.c \
			../src/RTspace.S \
			../src/RTstatic_ctors_dtors-linux-x86.c \
			../src/RTtlsgetaddr-linux-x86_64.S
#		   ../src/RTmutatedBinary.c \
#		   ../src/RTmutatedBinary_ELF.c \

#CFLAGS += -DDEBUG_PRINT_RT    # enable debug/trace messages from library

ifdef DETACH_ON_THE_FLY
CFLAGS += -DDETACH_ON_THE_FLY
SRCS += ../src/RTsigill-x86.S
endif

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# 32-bit runtime library
ASM_SRCS_32  = ../src/RTspace.S ../src/RTsignal-x86.S ../src/RTtlsgetaddr-linux-x86.S

TARGET_32_SO = libdyninstAPI_RT_m32.so.1
XTARGET_32   = $(basename $(basename $(TARGET_32_SO)))
VOUCHER_32   = V_$(XTARGET_32)
OBJS_32      = $(patsubst %.c, %_m32.o, $(filter %.c,$(notdir $(SRCS))))
ASM_OBJS_32  = $(patsubst %.S, %_m32.o, $(filter %.S,$(notdir $(ASM_SRCS_32))))
CFLAGS_32    = $(subst -DMUTATEE64,-DMUTATEE_32,$(CFLAGS))
LDFLAGS_32   = $(LDFLAGS)
LIBS_32      = -L/usr/lib $(LIBS)


$(OBJS_32): %_m32.o: ../src/%.c
	@echo "Compiling $< (32-bit)"
	$(HIDE_COMP)$(CC) $(subst -m64,-m32,$(CFLAGS_32)) -c $< -o $@
$(ASM_OBJS_32): %_m32.o: ../src/%.S
	@echo "Compiling $< (32-bit)"
	$(HIDE_COMP)$(CC) $(subst -m64,-m32,$(CFLAGS_32)) -c $< -o $@
$(VOUCHER_32):
	@echo Updating build voucher information: $(VOUCHER_32)
	$(HIDE_COMP)$(BUILDSTAMP) $(BUILD_ID) $(XTARGET)
	$(HIDE_COMP)$(MODCC) -m32 -c $(VOUCHER).c -o $(VOUCHER_32).o
$(TARGET_32_SO): $(OBJS_32) $(ASM_OBJS_32)
	$(HIDE_COMP)$(RM) $(TARGET_32_SO)
	$(HIDE_COMP)$(MAKE) $(VOUCHER_32)
	$(HIDE_COMP)$(LD) $(subst -m64,-m32,$(LDFLAGS_32)) -o $(TARGET_32_SO) $(VOUCHER_32).o $(OBJS_32) $(ASM_OBJS_32) $(LIBS_32)
RTspace.o: ../src/RTspace.S
	@echo "Compiling RTspace"
	$(HIDE_COMP)$(CC) $(CFLAGS) -c $< -o $@

$(DEST)/$(TARGET_32_SO): $(TARGET_32_SO)
	-$(CP) $(TARGET_32_SO) $(DEST)

# 32-bit static runtime library

STATIC_TARGET_32 = libdyninstAPI_RT_m32.a
STATIC_OBJS_32      = $(patsubst %.c, %_nopic_m32.o, $(filter %.c,$(notdir $(SRCS))))
STATIC_ASM_OBJS_32  = $(patsubst %.S, %_nopic_m32.o, $(filter %.S,$(notdir $(ASM_SRCS_32))))
STATIC_CFLAGS_32    =  $(subst -DMUTATEE64,-DMUTATEE_32,$(STATIC_CFLAGS))

$(STATIC_OBJS_32): %_nopic_m32.o: ../src/%.c
	@echo "Compiling $< (static 32-bit)"
	$(HIDE_COMP)$(CC) $(subst -m64,-m32,$(STATIC_CFLAGS_32)) -c $< -o $@
$(STATIC_ASM_OBJS_32): %_nopic_m32.o: ../src/%.S
	@echo "Compiling $< (static 32-bit)"
	$(HIDE_COMP)$(CC) $(subst -m64,-m32,$(STATIC_CFLAGS_32)) -c $< -o $@
$(STATIC_TARGET_32): $(STATIC_OBJS_32) $(STATIC_ASM_OBJS_32)
	$(HIDE_COMP)$(RM) $(STATIC_TARGET_32)
	$(HIDE_COMP)$(AR) rcs $(STATIC_TARGET_32) $(STATIC_OBJS_32) $(STATIC_ASM_OBJS_32)

$(DEST)/$(STATIC_TARGET_32): $(STATIC_TARGET_32)
	-$(CP) $(STATIC_TARGET_32) $(DEST)

ifndef SKIP_BUILD_RTLIB_32
install: $(DEST)/$(TARGET_32_SO) $(DEST)/$(STATIC_TARGET_32)
all: $(TARGET_32_SO) $(STATIC_TARGET_32)
endif

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

# This is just a dirty hack to make the build process complete
# smoothly.  It should be removed when we can correctly build
# a 64-bit runtime library.
#$(TARGET3_SO):
#	touch $(TARGET3_SO)
