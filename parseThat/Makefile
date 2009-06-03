
all: parsethat 
parsethat: $(PLATFORM)/Makefile
	cd $(PLATFORM); $(MAKE);
$(PLATFORM)/Makefile: $(PLATFORM)
	cd $(PLATFORM); \
	../configure --prefix=$(DYNINST_ROOT)/$(PLATFORM)/bin; \
	mv pMakefile Makefile
$(PLATFORM):
	mkdir $(PLATFORM);
clean:
	cd $(PLATFORM); $(MAKE) clean;
distclean:
	rm -rf $(PLATFORM)
install:
	cd $(PLATFORM); $(MAKE) install;
