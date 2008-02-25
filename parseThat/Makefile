# ----------------------------------------------
# Configuration variables taken from ./configure
# ----------------------------------------------
CPPFLAGS     = -I$(DYNINST_ROOT)/core/dynutil/h -I$(DYNINST_ROOT)/core/dyninstAPI/h -I$(DYNINST_ROOT)/core/symtabAPI/h -I$(DYNINST_ROOT)/core -g
CXX          = g++
CXXFLAGS     = -I$(DYNINST_ROOT)/core/dynutil/h -I$(DYNINST_ROOT)/core/dyninstAPI/h -I$(DYNINST_ROOT)/core/symtabAPI/h -I$(DYNINST_ROOT)/core -g
DEFINES      = -DPACKAGE_NAME=\"parseThat\" -DPACKAGE_TARNAME=\"parsethat\" -DPACKAGE_VERSION=\"1.0\" -DPACKAGE_STRING=\"parseThat\ 1.0\" -DPACKAGE_BUGREPORT=\"\" -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_BPATCH_PROCESS_H=1 
LDFLAGS      = -L$(DYNINST_ROOT)/$(PLATFORM)/lib
LIBS         = 

prefix       = .
PLATFORM    ?= i686-pc-linux-gnu
DYNINST_VER  = 5

INSTALL      =
#INSTALL      = install -c

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

OBJDIR       = obj-$(PLATFORM)
OBJS         = $(addprefix $(OBJDIR)/, $(SRCS:%.C=%.o))

CXXFLAGS    += -Wall
LIBS        += -ldyninstAPI -lsymtabAPI -liberty

ifneq (, $(findstring sparc, $(PLATFORM)))
LIBS        += -lelf -lstdc++ -lsocket -lrt
endif

ifneq (, $(findstring alpha, $(PLATFORM)))
CXXFLAGS    += -pthread
endif

ifneq (, $(findstring ia64, $(PLATFORM)))
LIBS        += -ldl
endif

all: ready $(OBJDIR)/parseThat

install: $(OBJDIR)/parseThat
	$(INSTALL) -d $(prefix)
	$(INSTALL) $(OBJDIR)/parseThat $(prefix)/parseThat

clean:
	rm -rf $(OBJDIR)

distclean: clean
	rm -rf config.log config.status $(OBJDIR) Makefile

ready:
	mkdir -p obj-$(PLATFORM)
	$(INSTALL) -d $(OBJDIR)

$(OBJDIR)/parseThat: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)

$(OBJS): $(OBJDIR)/%.o: %.C
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DEFINES) $^ -c -o $@

alarm: alarm.c liblib.so
	gcc -Wall -g -o alarm alarm.c liblib.so

liblib.so: lib.c
	gcc -Wall -g -shared -o liblib.so lib.c
