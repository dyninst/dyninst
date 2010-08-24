#
# TopLevel Makefile for the Paradyn (and DyninstAPI) system.
#
# $Id: Makefile,v 1.95 2008/10/03 21:12:39 legendre Exp $
#

TO_CORE = .
# Include additional local definitions (if available)
include ./make.config.local
# Include the make configuration specification (site configuration options)
include ./make.config

BUILD_ID = "$(SUITE_NAME) v$(RELEASE_NUM)$(BUILD_MARK)$(BUILD_NUM)"

SymtabAPI 	= ready common symtabAPI dynutil
StackwalkerAPI = ready common symtabAPI stackwalk
DyninstAPI	= ready common symtabAPI instructionAPI parseAPI dyninstAPI_RT dyninstAPI dynutil
InstructionAPI	= ready common instructionAPI dynutil
ProcControlAPI = ready common proccontrol
DepGraphAPI = depGraphAPI
ParseAPI = ready common symtabAPI instructionAPI parseAPI
ValueAdded = valueAdded/sharedMem
#DataflowAPI = instructionAPI parseAPI dataflowAPI
DataflowAPI = ParseAPI

testsuites = dyninstAPI/tests 
allSubdirs_noinstall =

ifndef DONT_BUILD_DYNINST
fullSystem	+= $(DyninstAPI)
Build_list	+= DyninstAPI
endif

ifndef DONT_BUILD_PROCCONTROL
fullSystem += proccontrol
Build_list += proccontrol
endif

ifndef DONT_BUILD_NEWTESTSUITE
testsuites += testsuite parseThat
allSubdirs_noinstall += testsuite 
fullSystem += testsuite parseThat
Build_list += testsuite parseThat
endif

fullSystem += parseAPI

allCoreSubdirs	= dyninstAPI_RT common dyninstAPI symtabAPI dynutil instructionAPI parseAPI
allSubdirs	= $(allCoreSubdirs) parseThat testsuites valueAdded/sharedMem depGraphAPI stackwalk proccontrol

# We're not building the new test suite on all platforms yet


# Note that the first rule listed ("all") is what gets made by default,
# i.e., if make is given no arguments.  Don't add other targets before all!

all: ready world

# This rule makes most of the normal recursive stuff.  Just about any
# target can be passed down to the lower-level Makefiles by listing it
# as a target here:

depend:
	+@for subsystem in $(fullSystem); do 			\
	    if [ -f $$subsystem/$(PLATFORM)/Makefile ]; then	\
			$(MAKE) -C $$subsystem/$(PLATFORM) $@;		\
	    else						\
			true;						\
	    fi							\
	done

clean distclean:
	+@for subsystem in $(allSubdirs); do 			\
	    if [ -f $$subsystem/$(PLATFORM)/Makefile ]; then	\
			$(MAKE) -C $$subsystem/$(PLATFORM) $@;		\
	    else						\
			true;						\
	    fi							\
	done

install:	ready world
	+@for subsystem in $(fullSystem); do 
			\
		if [ -f $$subsystem/$(PLATFORM)/Makefile ]; then	\
			$(MAKE) -C $$subsystem/$(PLATFORM) install;		\
		elif [ -f $$subsytem/Makefile ]; then 			\
			$(MAKE) -C $$subsystem install; \
	   else	\
			true;						\
	   fi							\
	done	

# Check that the main installation directories, etc., are ready for a build,
# creating them if they don't already exist!  Also touch make.config.local
# if needed.

ready:
	+@for installdir in $(LIBRARY_DEST) $(PROGRAM_DEST) $(INCLUDE_DEST); do \
	    if [ -d $$installdir ]; then			\
		echo "Installation directory $$installdir exists...";	\
	        true;						\
	    else						\
		echo "Creating installation directory $$installdir ...";\
	        mkdir -p $$installdir;				\
	    fi							\
	done

	+@if [ ! -f make.config.local ]; then	\
	    touch make.config.local;		\
	fi

	+@echo "Primary compiler for Paradyn build is:"
	+@if [ `basename $(CXX)` = "xlC" ]; then		\
               echo "xlC"; \
        elif [ `basename $(CXX)` = "insure" ]; then \
		echo "insure"; \
	else \
	echo CXX = $(CXX); \
	  $(CXX) -v; \
     true; \
	fi

# The "make world" target is set up to build things in the "correct"
# order for a build from scratch.  It builds and installs things in the
# "basicComps" list first, then builds and installs the remaining
# Paradyn "subSystems" (excluding the currently optional DyninstAPI).
# NB: "make world" has also been set up locally to build the DyninstAPI,
# however, this is optional and can be removed if desired.
#
# This make target doesn't go first in the Makefile, though, since we
# really only want to make "install" when it's specifically requested.
# Note that "world" doesn't do a "clean", since it's nice to be able
# to restart a compilation that fails without re-doing a lot of
# unnecessary work.

intro:
	@echo "Build of $(BUILD_ID) starting for $(PLATFORM)!"
ifdef DONT_BUILD_DYNINST
	@echo "Build of DyninstAPI components skipped!"
endif

world: intro
	$(MAKE) $(fullSystem)
	@echo "Build of $(BUILD_ID) complete for $(PLATFORM)!"

# "make Paradyn" and "make DyninstAPI" are also useful and valid build targets!

DyninstAPI SymtabAPI StackwalkerAPI basicComps subSystems testsuites InstructionAPI ValueAdded DepGraphAPI ParseAPI DataflowAPI ProcControlAPI: 
	$(MAKE) $($@)
	@echo "Build of $@ complete."
	@date

# Low-level directory-targets  (used in the sets defined above)
# Explicit specification of these rules permits better parallelization
# than building subsystems using a for loop

.PHONY: $(allSubdirs) $(allSubdirs_noinstall)

$(allSubdirs): 
	@if [ -f $@/$(PLATFORM)/Makefile ]; then \
		$(MAKE) -C $@/$(PLATFORM) && \
		$(MAKE) -C $@/$(PLATFORM) install; \
	elif [ -f $@/Makefile ]; then \
		$(MAKE) -C $@ && \
		$(MAKE) -C $@ install; \
	else \
		echo $@ has no Makefile; \
		true; \
	fi

$(allSubdirs_noinstall):
	+@if [ -f $@/$(PLATFORM)/Makefile ]; then \
		$(MAKE) -C $@/$(PLATFORM); \
	elif [ -f $@/Makefile ]; then \
		$(MAKE) -C $@; \
	else \
		echo $@ has no Makefile; \
		false; \
	fi

# Generate targets of the form install_<target> for all directories in
# allSubdirs_noinstall
allSubdirs_explicitInstall = $(patsubst %,install_%,$(allSubdirs_noinstall))
coreSubdirs_explicitInstall = $(patsubst %,install_%,$(allCoreSubdirs))

$(allSubdirs_explicitInstall): install_%: %
	+@if [ -f $(@:install_%=%)/$(PLATFORM)/Makefile ]; then \
		$(MAKE) -C $(@:install_%=%)/$(PLATFORM) install; \
	elif [ -f $(@:install_%=%)/Makefile ]; then \
		$(MAKE) -C $(@:install_%=%) install; \
	else \
		echo $(@:install_%=%) has no Makefile; \
		true; \
	fi


$(coreSubdirs_explicitInstall): install_%: %
	+@if [ -f $(@:install_%=%)/$(PLATFORM)/Makefile ]; then \
		$(MAKE) -C $(@:install_%=%)/$(PLATFORM) install; \
	elif [ -f $(@:install_%=%)/Makefile ]; then \
		$(MAKE) -C $(@:install_%=%) install; \
	else \
		echo $(@:install_%=%) has no Makefiles; \
		true; \
	fi
# dependencies -- keep parallel make from building out of order
symtabAPI igen: common
stackwalk: symtabAPI dynutil
dyninstAPI: symtabAPI instructionAPI parseAPI common dynutil
instructionAPI: common dynutil
symtabAPI dyninstAPI: dynutil
dyner codeCoverage dyninstAPI/tests testsuite: dyninstAPI
testsuite: $(coreSubdirs_explicitInstall)
testsuite: parseThat
parseThat: $(coreSubdirs_explicitInstall)
proccontrol: common dynutil
parseAPI: symtabAPI instructionAPI common dynutil
#depGraphAPI: instructionAPI $(coreSubdirs_explicitInstall)
# depGraphAPI: instructionAPI dyninstAPI

# This rule passes down the documentation-related make stuff to
# lower-level Makefiles in the individual "docs" directories.

docs install-man:
	+for subsystem in $(fullSystem); do			\
	    if [ -f $$subsystem/docs/Makefile ]; then		\
		$(MAKE) -C $$subsystem/docs $@;			\
	    else						\
		true;						\
	    fi							\
	done


# The "make nightly" target is what should get run automatically every
# night.  It uses "make world" to build things in the right order for
# a build from scratch.  
#
# Note that "nightly" should only be run on the primary build site,
# and does things like building documentation that don't need to be
# built for each different architecture.  Other "non-primary" build
# sites that run each night should just run "make clean world".

umd-nightly:
	$(MAKE) clean
	$(MAKE) DyninstAPI ValueAdded

# Used for UW nightly builds
nightly: all ValueAdded
	$(MAKE) -C testsuite/$(PLATFORM) all

#nightly:
#	$(MAKE) clean
#	$(MAKE) world
##	$(MAKE) DyninstAPI
#	$(MAKE) docs
#	$(MAKE) install-man
#	chmod 644 /p/paradyn/man/man?/*
