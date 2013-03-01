TO_CORE = ..

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
ifeq (xtrue,x$(LIBDWARF_STATIC))
LIBDWARF_LINK_FLAGS += -Wl,--whole-archive
endif

LIBDWARF_LINK_FLAGS += -ldwarf

ifeq (xtrue,x$(LIBDWARF_STATIC))
LIBDWARF_LINK_FLAGS += -Wl,--no-whole-archive
endif

endif

VPATH = $(PLATFORM)

ABS_PROGRAM_DEST := $(abspath $(PROGRAM_DEST))
ABS_LIBRARY_DEST := $(abspath $(LIBRARY_DEST))
ABS_INCLUDE_DEST := $(abspath $(INCLUDE_DEST))
ABS_LIBDWARF_DEST := $(LIBDWARF_LINK_FLAGS)
ABS_LIBELF_DEST := $(LIBELF_LINK_FLAGS)

ifdef includedir
CONFIG_ARGS = "--with-dyninst-include=$(includedir)"
endif
ifdef includedir
CONFIG_ARGS += "--with-dyninst-lib=$(libdir)"
endif
CONFIG_ARGS += "--with-dwarf-lib=$(ABS_LIBDWARF_DEST)"
ifdef LIBELF_LIB
CONFIG_ARGS += "--with-elf-lib=$(ABS_LIBELF_DEST)"
endif



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

MAKE_LOCAL_LOCATION = $(wildcard $(TO_CORE)/$(PLATFORM)/make.config.local)
ifndef MAKE_LOCAL_LOCATION
MAKE_LOCAL_LOCATION = $(TO_CORE)/make.config.local
endif

all:  $(PLATFORM)/parseThat

$(PLATFORM)/parseThat: $(PLATFORM)/Makefile
	$(MAKE) -C $(PLATFORM)

$(PLATFORM)/Makefile: Makefile.in $(MAKE_LOCAL_LOCATION)
	@if [ -d $(PLATFORM) ]; then \
		echo "directory $(PLATFORM) exists"; \
	else \
		mkdir $(PLATFORM); \
	fi; \
	cd $(PLATFORM); \
	echo $(CONFIG_ARGS); \
	CPPFLAGS="$(CPPFLAGS)" LDFLAGS="$(CONFIG_LD_FLAGS)" CXXFLAGS="$(CONFIG_CFLAGS)" CFLAGS="$(CONFIG_CFLAGS)" PLATFORM="$(PLATFORM)" ../configure $(CONFIG_ARGS) $(INSTALL_PREFIX);

clean:
	cd $(PLATFORM); $(MAKE) clean;

distclean:
	rm -rf $(PLATFORM)

install: $(PLATFORM)/parseThat
	cd $(PLATFORM); $(MAKE) install;
