
TO_SCRIPTS = ../../scripts
USES_DWARF_DEBUG = true
USES_LIBELF = true

include ../make.config
VPATH = $(PLATFORM)

ABS_PROGRAM_DEST = `cd $(PROGRAM_DEST); pwd`

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
	 	../configure --with-dyninst-lib=$(LIBRARY_DEST) --with-dyninst-include=$(INCLUDE_DEST) --prefix=$(ABS_PROGRAM_DEST); \
   else \
	 	../configure --with-dyninst-lib=$(LIBRARY_DEST) --with-dyninst-include=$(INCLUDE_DEST); \
   fi; \

clean:
	cd $(PLATFORM); $(MAKE) clean;

distclean:
	rm -rf $(PLATFORM)

install:
	cd $(PLATFORM); $(MAKE) install;
