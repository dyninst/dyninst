SUBDIRS = i386-unknown-linux2.4 rs6000-ibm-aix5.1 x86_64-unknown-linux2.4 ppc32_linux ppc64_linux x86_64_cnl i386-unknown-freebsd7.2 amd64-unknown-freebsd7.2 ppc32_bgp_ion ppc32_bgp ppc64_bgq_ion
SUBDIR_WINDOWS = i386-unknown-nt4.0

TO_SPEC = src/specification
PROLOG_FILES = $(TO_SPEC)/spec-grouped.pl $(TO_SPEC)/util.pl $(TO_SPEC)/test.pl
PYTHON_FILES = $(TO_SPEC)/makemake.py $(TO_SPEC)/tuples.py

.PHONY: usage gen-all echo $(SUBDIRS) $(SUBDIR_WINDOWS)

usage:
	@echo "Use target 'gen-all' to regenerate generated files for all supported"
	@echo "platforms"
	@echo "Use target 'gen-clean' to remove generated files for all supported"
	@echo "platforms"
	@echo "Use target PLATFORM to make for a specific platform"

GENERATED_FILES = make.mutators.gen make.solo_mutatee.gen test_info_new.gen.C
ALL_GENERATED_FILES = $(foreach dir,$(SUBDIRS),$(GENERATED_FILES:%=$(dir)/%))

GENERATED_FILES_WINDOWS = nmake.mutators.gen nmake.solo_mutatee.gen test_info_new.gen.C
ALL_GENERATED_FILES += $(GENERATED_FILES_WINDOWS:%=$(SUBDIR_WINDOWS)/%)

gen-all: $(ALL_GENERATED_FILES)

gen-clean:
	-rm -f $(ALL_GENERATED_FILES)


$(SUBDIRS:%=%/tuples): %/tuples: $(PROLOG_FILES)
	cd $(TO_SPEC); gprolog --entry-goal "['spec-grouped.pl']" \
		--entry-goal "test_init('$*')" \
		--entry-goal "write_tuples('../../$@', '$*')" \
		--entry-goal "halt"

$(SUBDIRS:%=%/make.mutators.gen): %/make.mutators.gen: $(PYTHON_FILES) %/tuples
	python -c "import sys; import os; os.environ['PLATFORM'] = '$*'; sys.path.append('$(TO_SPEC)'); import makemake; makemake.write_make_mutators_gen('$*/make.mutators.gen', '$*/tuples')"

$(SUBDIRS:%=%/make.solo_mutatee.gen): %/make.solo_mutatee.gen: $(PYTHON_FILES) %/tuples
	python -c "import sys; import os; os.environ['PLATFORM'] = '$*'; sys.path.append('$(TO_SPEC)'); import makemake; makemake.write_make_solo_mutatee_gen('$*/make.solo_mutatee.gen', '$*/tuples')"

# Generate the lists of tests to run in test_info_new.gen.C
$(SUBDIRS:%=%/test_info_new.gen.C): %/test_info_new.gen.C: $(PYTHON_FILES) %/tuples
	python -c "import sys; import os; os.environ['PLATFORM'] = '$*'; sys.path.append('$(TO_SPEC)'); import makemake; makemake.write_test_info_new_gen('$*/test_info_new.gen.C', '$*/tuples')"
	python -c "import sys; import os; os.environ['PLATFORM'] = '$*'; sys.path.append('$(TO_SPEC)'); import makemake; makemake.write_group_mutatee_boilerplate('$*/', '_group.c', '$*/tuples')";

$(SUBDIRS:%=%): %:%/make.mutators.gen %/make.solo_mutatee.gen %/test_info_new.gen.C

$(SUBDIR_WINDOWS:%=%): %:%/make.mutators.gen %/make.solo_mutatee.gen %/test_info_new.gen.C

# Do the same as above for windows
$(SUBDIR_WINDOWS:%=%/tuples): %/tuples: $(PROLOG_FILES)
	cd $(TO_SPEC); gprolog --entry-goal "['spec-grouped.pl']" \
		--entry-goal "test_init('$*')" \
		--entry-goal "write_tuples('../../$@', '$*')" \
		--entry-goal "halt"

$(SUBDIR_WINDOWS:%=%/nmake.mutators.gen): %/nmake.mutators.gen: $(PYTHON_FILES) %/tuples
	python -c "import sys; import os; os.environ['PLATFORM'] = '$*'; sys.path.append('$(TO_SPEC)'); import makemake; makemake.write_make_mutators_gen_nt('$*/nmake.mutators.gen', '$*/tuples')"

$(SUBDIR_WINDOWS:%=%/nmake.solo_mutatee.gen): %/nmake.solo_mutatee.gen: $(PYTHON_FILES) %/tuples
	python -c "import sys; import os; os.environ['PLATFORM'] = '$*'; sys.path.append('$(TO_SPEC)'); import makemake; makemake.write_make_solo_mutatee_gen_nt('$*/nmake.solo_mutatee.gen', '$*/tuples')"

#TODO do I need this big substitution thing here?
$(SUBDIR_WINDOWS:%=%/test_info_new.gen.C): %/test_info_new.gen.C: $(PYTHON_FILES) %/tuples
	python -c "import sys; import os; os.environ['PLATFORM'] = '$*'; sys.path.append('$(TO_SPEC)'); import makemake; makemake.write_test_info_new_gen_nt('$*/test_info_new.gen.C', '$*/tuples')"
