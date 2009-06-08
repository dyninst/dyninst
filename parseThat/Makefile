VPATH = $(PLATFORM)
all:  $(PLATFORM)/parseThat
$(PLATFORM)/parseThat: $(PLATFORM)/Makefile
	$(MAKE) -C $(PLATFORM)
$(PLATFORM)/Makefile: pMakefile.in
	@if [ -d $(PLATFORM) ]; then \
		echo "directory $(PLATFORM) exists"; \
	else \
		mkdir $(PLATFORM); \
	fi
	cd $(PLATFORM); \
	../configure --prefix=$(DYNINST_ROOT)/$(PLATFORM)/bin; \
	mv pMakefile Makefile
clean:
	cd $(PLATFORM); $(MAKE) clean;
distclean:
	rm -rf $(PLATFORM)
install:
	cd $(PLATFORM); $(MAKE) install;
