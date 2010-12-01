#
# $Id: Makefile,v 1.12 2008/02/20 08:31:08 jaw Exp $
#

# Define any symbols needed to invoke configuration changes in make.config

USES_SHM_SAMPLING = true

# We set the permissions of the runtime library as 640
OVERRIDE_INSTALL = true
DEBUG_FLAG = -g

# AIX: libraries end in .a
TARGET = libdyninstAPI_RT.a
TARGET_SYMLINK = libdyninstAPI_RT.so

# Include standard make configuration stuff that applies to everything
# in the paradyn tree.

include               ../../make.config

CFLAGS	+= -Wa,-mppc
#CFLAGS	+= -DDEBUG_PRINT_RT     # enable debug/trace messages from library
#CFLAGS += -DUSE_PROF
ASFLAGS += -I../..

SRCS  += ../src/RTposix.c \
	../src/RTaix.c \
	../src/RTheap.c \
	../src/RTheap-aix.c \
	../src/RTthread-power-asm.s \
	../src/RTthread-power.c\
	../src/RTthread-index.c \
	../src/RTthread.c
#	../src/RTmutatedBinary.c \
#	../src/RTmutatedBinary_XCOFF.c \

# Build and link the library text heap
ifdef USES_LIB_TEXT_HEAP
EXTRA_LIBS = libDyninstText.a
CFLAGS	+= -DUSES_LIB_TEXT_HEAP
LDFLAGS += -L. -lDyninstText
endif

LDFLAGS +=-Wl,-bM:SRE -Wl,-bexpall -Wl,-bnoobjreorder -lc -Wl,-bgcbypass:5
#-Wl,-bnosymbolic
#-Wl,-brtl 
#-Wl,-bnortllib -Wl,-bnosymbolic

#erok, rtl, nortllib, nosymbolic, noautoexp == -G

# Include the module-specific Makefile, which defines everything about
# the module that is common across architectures.

include ../make.module.tmpl

# Include the "standard program template".  This defines all the
# common targets like "clean", "install", etc.

include ../../make.library.tmpl

# If we're not using the native compiler, use gcc as a linker
ifdef USES_NATIVE_CC
LD		= $(LINKER) 
LDFLAGS		+= -bnoobjreorder -bexpall -bnoentry  -lc
# Initialization method
LDFLAGS		+= -binitfini:libdyninstAPI_RT_init

libDyninstText.a: space.o
#	rm -f libSpace.o 
#	rm -f libDyninstText.a 
	ld -o space_lib.o -bexpall -bnoentry -bM:SRE space.o 
	ar crv libDyninstText.a space_lib.o 
	rm -f space_lib.o
#	rm -f space.o


else
# GCC
CC = gcc
LD		= $(GCC) -shared
# Initialization method
LDFLAGS		+= -Wl,-binitfini:libdyninstAPI_RT_init

libDyninstText.a: space.o
	gcc -shared -o libDyninstText.a space.o

endif


libDyninstAPI_RT.so: libDyninstText.a
# ... and uses default AIX link line

RTthread-power-asm.o:
	$(GCC) -c $(CFLAGS) ../src/RTthread-power-asm.s

space.o: 
	$(GCC) -c -o space.o ../src/libSpace.s

install: install_generic libDyninstText.a
	-chmod 640 $(DEST)/$(TARGET)
	-$(CP) libDyninstText.a $(DEST)
	-chmod 755 $(DEST)/libDyninstText.a
	-ln -sf $(TARGET) $(DEST)/$(TARGET_SYMLINK)

#
# 32-bit runtime library
#
ASM_SRCS_32  = ../src/RTthread-power-asm.s

TARGET_32    = libdyninstAPI_RT_m32.a
TARGET_SYMLINK_32 = libdyninstAPI_RT_m32.so

XTARGET_32   = $(basename $(TARGET_32))
VOUCHER_32   = V_$(XTARGET_32)
OBJS_32      = $(patsubst %.c, %_m32.o, $(filter %.c,$(notdir $(SRCS))))
ASM_OBJS_32  = $(patsubst %.s, %_m32.o, $(filter %.s,$(notdir $(ASM_SRCS_32))))

CFLAGS_32_a  = -DMUTATEE_32 $(CFLAGS)
CFLAGS_32_b  = $(subst -maix64,-maix32,$(CFLAGS_32_a))
CFLAGS_32_c  = $(subst -Darch_64bit,,$(CFLAGS_32_b))
CFLAGS_32    = $(subst -Dcap_32_64,,$(CFLAGS_32_c))

LDFLAGS_32   = $(subst -maix64,-maix32,$(LDFLAGS))
LIBS_32      = -L/usr/lib $(LIBS)

$(OBJS_32): %_m32.o: ../src/%.c
	$(CC) $(CFLAGS_32) -c $< -o $@
$(VOUCHER_32):
	@echo Updating build voucher information: $(VOUCHER_32)
	$(BUILDSTAMP) $(BUILD_ID) $(XTARGET)
	$(MODCC) $(CFLAGS_32) -c $(VOUCHER).c -o $(VOUCHER_32).o
$(TARGET_32): $(OBJS_32) $(ASM_OBJS_32)
	@$(RM) $(TARGET_32)
	@$(MAKE) $(VOUCHER_32)
	$(LD) $(LDFLAGS_32) -o $(TARGET_32) $(VOUCHER_32).o $(OBJS_32) $(ASM_OBJS_32) $(LIBS_32)

RTthread-power-asm_m32.o: ../src/RTthread-power-asm.s
	$(GCC) -c $(CFLAGS_32) -o RTthread-power-asm_m32.o ../src/RTthread-power-asm.s

space_m32.o: ../src/libSpace.s
	$(GCC) -c $(CFLAGS_32) -o space_m32.o ../src/libSpace.s

libDyninstText_m32.a: space_m32.o
	gcc -shared $(CFLAGS_32) -o libDyninstText_m32.a space_m32.o

$(DEST)/$(TARGET_32): $(TARGET_32) libDyninstText_m32.a
	-$(CP) $(TARGET_32) $(DEST)
	-chmod 755 $(DEST)/$(TARGET_32)
	-ln -sf $(TARGET_32) $(DEST)/$(TARGET_SYMLINK_32)

ifndef SKIP_BUILD_RTLIB_32
install: $(DEST)/$(TARGET_32)
all: $(TARGET_32)
endif
