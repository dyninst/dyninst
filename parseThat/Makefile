TO_CORE = ../
TO_SCRIPTS = ../../scripts

USES_DWARF_DEBUG = true
USES_LIBELF = true

include ../make.config

ifdef LIBELF_PLATFORM
ifneq (x,x$(strip $(LIBELF_LIB)))
LIBELF_LINK_FLAGS += -L$(LIBELF_LIB)
endif
LIBELF_LINK_FLAGS += -lelf
endif

ifdef LIBDWARF_PLATFORM
ifneq (x,x$(strip $(LIBDWARF_LIB)))
LIBDWARF_LINK_FLAGS += -L$(LIBDWARF_LIB)
endif
LIBDWARF_LINK_FLAGS += -ldwarf
endif

VPATH = $(PLATFORM)

ABS_PROGRAM_DEST := $(abspath $(PROGRAM_DEST))
ABS_LIBRARY_DEST := $(abspath $(LIBRARY_DEST))
ABS_INCLUDE_DEST := $(abspath $(INCLUDE_DEST))
ABS_LIBDWARF_DEST := $(LIBDWARF_LINK_FLAGS)
ABS_LIBELF_DEST := $(LIBELF_LINK_FLAGS)

#CONFIG_ARGS = "--with-dyninst-lib=$(TO_CORE)/$(ABS_LIBRARY_DEST)"
#CONFIG_ARGS += "--with-dyninst-include=$(ABS_INCLUDE_DEST)"
CONFIG_ARGS = "--with-dwarf-lib=$(ABS_LIBDWARF_DEST)"
CONFIG_ARGS += "--with-elf-lib=$(ABS_LIBELF_DEST)"

ifdef DEMANGLER_EXEC_LINK
CONFIG_ARGS += "--with-liberty-lib=\"$(DEMANGLER_EXEC_LINK)\""
endif

CONFIG_LD_FLAGS = $(patsubst %,-L../%,$(shell echo ../*/$(PLATFORM)))
CONFIG_CFLAGS = $(patsubst %,-I../%,$(shell echo ../*/h))

ifdef ABS_PROGRAM_DEST
INSTALL_PREFIX = --prefix=$(ABS_PROGRAM_DEST)
else
INSTALL_PREFIX =
endif

all:  $(PLATFORM)/parseThat

$(PLATFORM)/parseThat: $(PLATFORM)/Makefile
	$(MAKE) -C $(PLATFORM)

$(PLATFORM)/Makefile: Makefile.in
	@if [ -d $(PLATFORM) ]; then \
		echo "directory $(PLATFORM) exists"; \
	else \
		mkdir $(PLATFORM); \
	fi; \
   cd $(PLATFORM); \
	LDFLAGS="$(CONFIG_LD_FLAGS)" CXXFLAGS="$(CONFIG_CFLAGS)" CFLAGS="$(CONFIG_CFLAGS)" PLATFORM="$(PLATFORM)" ../configure $(CONFIG_ARGS) $(INSTALL_PREFIX);

clean:
	cd $(PLATFORM); $(MAKE) clean;

distclean:
	rm -rf $(PLATFORM)

install: $(PLATFORM)/parseThat
	cd $(PLATFORM); $(MAKE) install;
