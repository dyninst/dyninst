# ------------------------------------------------------------------------------
# User options
# ------------------------------------------------------------------------------

# Platform string, could be taken from environment.
PLATFORM ?=

# Location of compiler to use.
CXX = g++

# Build options.
DEFINES =
CXXFLAGS =
LDFLAGS =

# Location and version of DyninstAPI we are linking against.
DYNINST_VER = 5
DYNINST_DIR = $(DYNINST_ROOT)
DYNINST_INC = $(DYNINST_DIR)/core/dyninstAPI/h
DYNINST_LIB = $(DYNINST_DIR)/$(PLATFORM)/lib

# Location of OpenSSL package.  Used for calculating SHA1 sums.
OPENSSL_DIR = /usr
OPENSSL_INC = $(OPENSSL_DIR)/include
OPENSSL_LIB = $(OPENSSL_DIR)/lib

# End of user modifiable options.  Do not modify beyond this line.
# ==============================================================================

ifeq ($(PLATFORM),)
$(error PLATFORM environment variable must be defined)
endif

.PHONY: clean ready

SRCS = parseThat.C config.C ipc.C record.C strlist.C reglist.C log.C utils.C dyninstCore.C

ifeq ($(DYNINST_VER), 5)
SRCS += dyninstCompat.v5.C
endif

ifeq ($(DYNINST_VER), 4)
SRCS += dyninstCompat.v4.C
endif

OBJS=$(addprefix $(PLATFORM)/, $(SRCS:%.C=%.o))

DEFINES		+= -DDYNINST_VER=$(DYNINST_VER)
CXXFLAGS	+= -g -Wall -Isrc -I$(DYNINST_INC) -I$(DYNINST_DIR)/core -I$(OPENSSL_INC) $(DEFINES)
LDFLAGS		+= -L$(DYNINST_LIB) -ldyninstAPI -liberty -L$(OPENSSL_LIB) -lcrypto

ifneq (, $(findstring sparc, $(PLATFORM)))
LDFLAGS		+= -lelf -lstdc++ -lsocket -lrt
endif

ifneq (, $(findstring alpha, $(PLATFORM)))
CXXFLAGS	+= -pthread
endif

ifneq (, $(findstring ia64, $(PLATFORM)))
LDFLAGS		+= -ldl
endif

all: ready install

$(PLATFORM)/parseThat: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

install: $(PLATFORM)/parseThat
	rm -f parseThat
	ln -s $(PLATFORM)/parseThat parseThat

clean:
	rm -rf $(PLATFORM)

ready:
	if [ ! -d $(PLATFORM) ]; then mkdir $(PLATFORM); fi

$(OBJS): $(PLATFORM)/%.o: src/%.C
	$(CXX) $(CXXFLAGS) $^ -c -o $@

alarm: alarm.c liblib.so
	gcc -Wall -g -o alarm alarm.c liblib.so

liblib.so: lib.c
	gcc -Wall -g -shared -o liblib.so lib.c
