#
# TopLevel Makefile for the Paradyn (and DyninstAPI) system.
#
# $Id: Makefile,v 1.95 2008/10/03 21:12:39 legendre Exp $
#

TO_CORE = .

# Include the make configuration specification (site configuration options)
include ./make.config

# Include component dependency information
include ./make.components

all: world

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

test-full:
	@$(MAKE) -C testsuite/$(PLATFORM) full

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
ifndef DONT_BUILD_NEWTESTSUITE
dyn_testsuite = dyninstAPI_testsuite
sym_testsuite = symtabAPI_testsuite
ins_testsuite = instructionAPI_testsuite
pc_testsuite = proccontrol_testsuite
endif


DyninstAPI: dyninstAPI parseThat $(dyn_testsuite)
SymtabAPI: symtabAPI $(sym_testsuite)
StackwalkerAPI: stackwalk
basicComps:  dyninstAPI
subSystems:  dyninstAPI
testsuites:  testsuite
InstructionAPI:  instructionAPI $(ins_testsuite)
ValueAdded: valueAdded/sharedMem
DynC_API: dynC_API
ParseAPI: parseAPI
DataflowAPI: parseAPI
ProcControlAPI: proccontrol $(pc_testsuite)
PatchAPI: patchAPI

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

echo:
	@echo $(Everything)

PKGCONFIG_CFLAGS = $(CXX_REQ_FLAGS) -I$(includedir)
PKGCONFIG_LIBS = -L$(libdir) -ldyninstAPI -lstackwalk -lpcontrol -lpatchAPI -lparseAPI -linstructionAPI -lsymtabAPI -lsymLite -ldynDwarf -ldynElf -lcommon 

ifdef LIBDWARF_PLATFORM
PKGCONFIG_CFLAGS += -I$(LIBDWARF_INC)
ifeq (xtrue,x$(LIBDWARF_STATIC))
PKGCONFIG_LIBS += -rdynamic -Wl,--whole-archive
endif
PKGCONFIG_LIBS += -L$(LIBDWARF_LIB) -ldwarf
ifeq (xtrue,x$(LIBDWARF_STATIC))
PKGCONFIG_LIBS += -Wl,--no-whole-archive
endif
endif

ifdef LIBELF_PLATFORM
PKGCONFIG_CFLAGS += -I$(LIBELF_INC)
PKGCONFIG_LIBS += -L$(LIBELF_LIB) -lelf
endif

ifdef USE_LIBIBERTY
ifdef LIBIBERTY_LIB
PKGCONFIG_LIBS += -L$(LIBIBERTY_LIB)
endif
PKGCONFIG_LIBS += -rdynamic -Wl,--whole-archive -liberty -Wl,--no-whole-archive
endif

pkgconfig:
	$(HIDE_COMP)$(RM) dyninst.pc
	$(HIDE_COMP)echo "# This file contains pkgconfig information for Dyninst and its components" > dyninst.pc
	$(HIDE_COMP)echo "prefix = $(prefix)" >> dyninst.pc
	$(HIDE_COMP)echo "exec_prefix = $(exec_prefix)" >> dyninst.pc
	$(HIDE_COMP)echo "includedir = $(includedir)" >> dyninst.pc
	$(HIDE_COMP)echo "libdir = $(libdir)" >> dyninst.pc
	$(HIDE_COMP)echo "bindir = $(bindir)" >> dyninst.pc
	$(HIDE_COMP)echo >> dyninst.pc
	$(HIDE_COMP)echo "Name: dyninst" >> dyninst.pc
	$(HIDE_COMP)echo "Description: DyninstAPI binary instrumentation library" >> dyninst.pc
	$(HIDE_COMP)echo "Requires: libelf, libdwarf" >> dyninst.pc
	$(HIDE_COMP)echo "Version: 8.1.1" >> dyninst.pc
	$(HIDE_COMP)echo "Cflags: $(PKGCONFIG_CFLAGS)" >> dyninst.pc
	$(HIDE_COMP)echo "Libs: $(PKGCONFIG_LIBS)" >> dyninst.pc
	$(HIDE_COMP)echo "Creating dyninst.pc pkgconfig file"
