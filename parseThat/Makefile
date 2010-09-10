TO_CORE = ../
TO_SCRIPTS = ../../scripts
USES_DWARF_DEBUG = true
USES_LIBELF = true

include ../make.config
VPATH = $(PLATFORM)

ABS_PROGRAM_DEST := $(shell cd $(PROGRAM_DEST); pwd)
ABS_LIBRARY_DEST := $(shell cd $(LIBRARY_DEST); pwd)
ABS_INCLUDE_DEST := $(shell cd $(INCLUDE_DEST); pwd)
ABS_LIBDWARF_DEST := $(shell cd $(LIBDWARF_LIB); pwd)
ABS_LIBELF_DEST := $(shell cd $(LIBELF_LIB); pwd)

CONFIG_ARGS = "--with-elf-lib=$(ABS_LIBELF_DEST)"
CONFIG_ARGS += "--with-dwarf-lib=$(ABS_LIBDWARF_DEST)"
CONFIG_ARGS += "--with-dyninst-lib=$(ABS_LIBRARY_DEST)"
CONFIG_ARGS += "--with-dyninst-include=$(ABS_INCLUDE_DEST)"

ifdef LINK_LIBERTY
	CONFIG_ARGS += "LDFLAGS=-liberty"
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
   if [ -d $(ABS_PROGRAM_DEST) ]; then \
	../configure $(CONFIG_ARGS) --prefix=$(ABS_PROGRAM_DEST); \
   else \
	../configure $(CONFIG_ARGS); \
   fi; \

clean:
	cd $(PLATFORM); $(MAKE) clean;

distclean:
	rm -rf $(PLATFORM)

install: $(PLATFORM)/parseThat
	cd $(PLATFORM); $(MAKE) install;
