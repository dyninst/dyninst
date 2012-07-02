#
# TopLevel Makefile for the Paradyn (and DyninstAPI) system.
#
# $Id: Makefile,v 1.95 2008/10/03 21:12:39 legendre Exp $
#

TO_CORE = .

# Include the make configuration specification (site configuration options)
include ./make.config

# Note that the first rule listed ("all") is what gets made by default,
# i.e., if make is given no arguments.  Don't add other targets before all!
all: world

# Include component dependency information
include ./make.components

.PHONY: $(Everything) $(Everything_install) $(Everything_tests) $(Everything_tests_install) install world intro comp_intro ready clean distclean depend all
.PHONY: DyninstAPI SymtabAPI StackwalkerAPI basicComps subSystems testsuites InstructionAPI ValueAdded DepGraphAPI ParseAPI DynC_API DataflowAPI ProcControlAPI PatchAPI

BUILD_ID = "$(SUITE_NAME) v$(RELEASE_NUM)$(BUILD_MARK)$(BUILD_NUM)"

$(Everything) $(Everything_tests):
	@if [ -f $@/$(PLATFORM)/Makefile ]; then \
		$(MAKE) -C $@/$(PLATFORM); \
	elif [ -f $@/Makefile ]; then \
		$(MAKE) -C $@; \
	else \
		echo $@ has no Makefile; \
		true; \
	fi

$(Everything_install) $(Everything_tests_install):
	@if [ -f $(@:%_install=%)/$(PLATFORM)/Makefile ]; then \
		$(MAKE) -C $(@:%_install=%)/$(PLATFORM) install; \
	elif [ -f $(@:%_install=%)/Makefile ]; then \
		$(MAKE) -C $(@:%_install=%) install; \
	else \
		echo $(@:%_install=%) has no Makefile; \
		true; \
	fi

$(Test_targets):
	@$(MAKE) -C testsuite/$(PLATFORM) $(@:%_testsuite=%)

install: intro ready $(fullSystem_install)

world: intro $(fullSystem)
depend:
	+@for subsystem in $(fullSystem); do 			\
	    if [ -f $$subsystem/$(PLATFORM)/Makefile ]; then	\
			$(MAKE) -C $$subsystem/$(PLATFORM) $@;		\
	    else						\
			true;						\
	    fi							\
	done

clean distclean:
	+@for subsystem in $(Everything) $(Everything_tests); do 			\
	    if [ -f $$subsystem/$(PLATFORM)/Makefile ]; then	\
			$(MAKE) -C $$subsystem/$(PLATFORM) $@;		\
	    else						\
			true;						\
	    fi							\
	done

# Check that the main installation directories, etc., are ready for a build,
# creating them if they don't already exist!
ready:
	@echo "[User:`whoami` Host:`hostname` Platform:$(PLATFORM) Date:`date '+%Y-%m-%d'`]"
	+@for installdir in $(LIBRARY_DEST) $(PROGRAM_DEST) $(INCLUDE_DEST); do \
	    if [ -d $$installdir ]; then			\
		echo "Installation directory $$installdir exists...";	\
	        true;						\
	    else						\
		echo "Creating installation directory $$installdir ...";\
	        $(INSTALL) -d -p $$installdir;				\
	    fi							\
	done

intro: comp_intro
	@echo "Build of $(BUILD_ID) on $(PLATFORM) for $(DEFAULT_COMPONENT): $(fullSystem)"

comp_intro:
	+@echo "Primary compiler for Paradyn build is:"
	+@if [ `basename $(CXX)` = "xlC" ]; then		\
     echo "xlC"; \
	else \
	echo CXX = $(CXX); \
	  $(CXX) --version; \
     true; \
	fi

# Before refactoring the Makefile, we used to support these build targets.  Recreate them with
# simple aliases 
DyninstAPI: comp_intro dyninstAPI parseThat dyninstAPI_testsuite
SymtabAPI: comp_intro symtabAPI symtabAPI_testsuite
StackwalkerAPI: comp_intro stackwalk
basicComps: comp_intro dyninstAPI
subSystems: comp_intro dyninstAPI
testsuites: comp_intro testsuite
InstructionAPI: comp_intro instructionAPI instructionAPI_testsuite
ValueAdded: comp_intro valueAdded/sharedMem
DepGraphAPI: comp_intro depGraphAPI
ParseAPI: comp_intro parseAPI
DynC_API: comp_intro dynC_API
DataflowAPI: comp_intro parseAPI
ProcControlAPI: comp_intro proccontrol proccontrol_testsuite
PatchAPI: comp_intro parseAPI

# Testsuite dependencies
parseThat: $(filter-out parseThat,$(parseThat))
parseThat_install: $(fullSystem_install_notests)
testsuite: $(fullSystem_notests)
testsuite_install: $(fullSystem_install_notests)

# For each dependency in make.components (targ), create a rule that looks like
#  targ: $filter-out( $(targ),$($(targ)))
# Thus when targ is stackwalk we will evaluate to:
#   stackwalk: dynutil common proccontrol    
$(foreach targ,$(Everything),$(eval $(targ): $(filter-out $(targ),$($(targ)))))

# Same as above, but for %_install targets
$(foreach targ,$(Everything),$(eval $(targ)_install: $(patsubst %,%_install,$(filter-out $(targ),$($(targ))))))

#Every install target depends on ready
$(foreach targ,$(Everything),$(eval $(targ)_install: ready))

# Now add testsuite dependency rules.  An example of these expanding is:
#   dyninstAPI_testsuite: dyninstAPI
$(foreach targ,$(test_comps),$(eval $(targ)_testsuite: $(targ)))

docs install-man:
	+for subsystem in $(fullSystem); do			\
	    if [ -f $$subsystem/docs/Makefile ]; then		\
		$(MAKE) -C $$subsystem/docs $@;			\
	    else						\
		true;						\
	    fi							\
	done


# The "make nightly" target is what should get run automatically every
# Note that the nightlies will build the testsuite with all mutatees
# at all optimization levels--this gets huge
testsuite-nightly:
	$(MAKE) -C testsuite/$(PLATFORM) all

umd-nightly:
	$(MAKE) clean
	$(MAKE) nightly

# Used for UW nightly builds
nightly: ready $(Everything_install) parseThat_install testsuite-nightly
