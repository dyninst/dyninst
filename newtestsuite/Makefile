SUBDIRS = i386-unknown-linux2.4 ia64-unknown-linux2.4 rs6000-ibm-aix5.1 x86_64-unknown-linux2.4 sparc-sun-solaris2.8 sparc-sun-solaris2.9 ppc32_linux

TO_SPEC = src/specification
PROLOG_FILES = spec.pl util.pl test.pl
PYTHON_FILES = $(TO_SPEC)/makemake.py $(TO_SPEC)/tuples.py

.PHONY: usage gen-all echo

usage:
	@echo "Use target 'gen-all' to regenerate generated files for all supported"
	@echo "platforms"
	@echo "Use target 'gen-clean' to remove generated files for all supported"
	@echo "platforms"

GENERATED_FILES = make.mutators.gen make.solo_mutatee.gen test_info_new.gen.C
ALL_GENERATED_FILES = $(foreach dir,$(SUBDIRS),$(GENERATED_FILES:%=$(dir)/%))

gen-all: $(ALL_GENERATED_FILES)

gen-clean:
	-rm $(ALL_GENERATED_FILES) $(PROLOG_FILES)

$(PROLOG_FILES): %: $(TO_SPEC)/%
	cp $(TO_SPEC)/$* .

$(SUBDIRS:%=%/tuples): %/tuples: $(PROLOG_FILES)
	gprolog --entry-goal "['$(TO_SPEC)/spec.pl']" \
		--entry-goal "test_init('$*')" \
		--entry-goal "write_tuples('$*/tuples', '$*')" \
		--entry-goal "halt"

$(SUBDIRS:%=%/make.mutators.gen): %/make.mutators.gen: $(PYTHON_FILES) %/tuples
	python -c "import sys; import os; os.environ['PLATFORM'] = '$*'; sys.path.append('$(TO_SPEC)'); import makemake; makemake.write_make_mutators_gen('$*/make.mutators.gen', '$*/tuples')"

$(SUBDIRS:%=%/make.solo_mutatee.gen): %/make.solo_mutatee.gen: $(PYTHON_FILES) %/tuples
	python -c "import sys; import os; os.environ['PLATFORM'] = '$*'; sys.path.append('$(TO_SPEC)'); import makemake; makemake.write_make_solo_mutatee_gen('$*/make.solo_mutatee.gen', '$*/tuples')"

# Generate the lists of tests to run in test_info_new.gen.C
$(SUBDIRS:%=%/test_info_new.gen.C): %/test_info_new.gen.C: $(PYTHON_FILES) %/tuples
	python -c "import sys; import os; os.environ['PLATFORM'] = '$*'; sys.path.append('$(TO_SPEC)'); import makemake; makemake.write_test_info_new_gen('$*/test_info_new.gen.C', '$*/tuples')"

