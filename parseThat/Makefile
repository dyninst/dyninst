# ----------------------------------------------
# Configuration variables taken from ./configure
# ----------------------------------------------
CPPFLAGS     = -I/scratch/bill/dyninst/include 
CXX          = g++
CXXFLAGS     = -g -O2
DEFINES      = -DPACKAGE_NAME=\"parseThat\" -DPACKAGE_TARNAME=\"parsethat\" -DPACKAGE_VERSION=\"1.0\" -DPACKAGE_STRING=\"parseThat\ 1.0\" -DPACKAGE_BUGREPORT=\"\" -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_BPATCH_PROCESS_H=1
LDFLAGS      = -L/scratch/bill/dyninst/lib 
DWARFFLAGS      = 
ELFFLAGS      = 
EXTRA_LIBS         = -ldl 
DEMANGLER_LINK = 
prefix       = /scratch/bill/dyninst
PLATFORM    ?= x86_64-unknown-linux-gnu
DYNINST_VER  = 5

INSTALL      = /usr/bin/install -c

vpath % ./src

# -------------------------------------------
# Begin Makefile based on variables set above
# -------------------------------------------
.PHONY: clean ready

SRCS         = parseThat.C \
               config.C \
               ipc.C \
               record.C \
               strlist.C \
               reglist.C \
               log.C \
               utils.C \
               sha1.C \
               dyninstCore.C

ifeq ($(DYNINST_VER), 5)
SRCS        += dyninstCompat.v5.C
endif

ifeq ($(DYNINST_VER), 4)
SRCS        += dyninstCompat.v4.C
endif

OBJS         = $(SRCS:%.C=%.o)

CXXFLAGS    += -Wall
LIBS        += -ldyninstAPI -lstackwalk -lpcontrol -lpatchAPI -lparseAPI -linstructionAPI -lsymtabAPI -lsymLite -ldynDwarf -ldynElf -lcommon -pthread

# The echo will remove the quotes
LIBS += $(shell echo $(DEMANGLER_LINK))

ifneq (x,x$(strip $(ELFFLAGS)))
LIBS += $(ELFFLAGS) 
endif
ifneq (x,x$(strip $(DWARFFLAGS)))
LIBS += $(DWARFFLAGS)
endif

ifeq (freebsd, $(findstring freebsd, $(PLATFORM)))
LIBS	    += -lpthread
endif

ifeq (ppc64_linux, $(findstring ppc64_linux, $(PLATFORM)))
CXXFLAGS	+= -m64
CFLAGS		+= -m64
endif

ifneq (x$(DESTDIR),x)
TARGETDIR = $(DESTDIR)/$(prefix)
else
TARGETDIR = $(prefix)
endif

LIBS += ${EXTRA_LIBS}

all: ready parseThat

install: $(TARGETDIR)/parseThat

$(TARGETDIR)/parseThat: parseThat
	@if [ $(TARGETDIR) != "." ]; then                           \
		echo "$(INSTALL) -d $(TARGETDIR)";                  \
		$(INSTALL) -d $(TARGETDIR);                         \
		echo "$(INSTALL) parseThat $(TARGETDIR)/parseThat"; \
		$(INSTALL) parseThat $(TARGETDIR)/parseThat;        \
	fi

clean:
	rm -f parseThat *.o *.so

distclean: clean
	rm Makefile config.log config.status

ready:
	@echo "Begin parseThat build for $(PLATFORM)"

parseThat: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)

$(OBJS): %.o: %.C
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DEFINES) $^ -c -o $@

alarm: alarm.c liblib.so
	gcc -Wall -g -o alarm alarm.c liblib.so

liblib.so: lib.c
	gcc -Wall -g -shared -o liblib.so lib.c
