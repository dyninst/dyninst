#
# TopLevel Makefile for the Paradyn (and DyninstAPI) system.
#
# $Id: Makefile,v 1.65 2004/06/24 22:27:49 legendre Exp $
#

# Include the make configuration specification (site configuration options)
include ./make.config
# Include additional local (re-)definitions (if available)
-include ./make.config.local

BUILD_ID = "$(SUITE_NAME) v$(RELEASE_NUM)$(BUILD_MARK)$(BUILD_NUM)"

# "basicComps" is the list of components which need to be built first
# with "make world", since they are used building the rest of the system.
#
# "subSystems" is the list of all other pieces which should be built
# as part of Paradyn.
#
# "DyninstAPI" is the list of additional API components (optional).

basicComps	= igen mrnet pdutil mdl
ParadynD	= dyninstAPI_RT sharedMem rtinst paradynd
ParadynFE	= mrnet pdutil pdthread paradyn
ParadynVC	= visi \
		visiClients/tclVisi visiClients/barchart \
		visiClients/tableVisi visiClients/phaseTable \
		visiClients/histVisi visiClients/terrain \
		visiClients/termWin
subSystems	= $(ParadynD) $(ParadynFE) $(ParadynVC)
DyninstAPI	= dyninstAPI_RT dyninstAPI dyninstAPI/tests dyner codeCoverage

threadComps	= rtinst/multi-thread-aware

# "fullSystem" is the list of all Paradyn & DyninstAPI components to build:
# set DONT_BUILD_PARADYN or DONT_BUILD_DYNINST in make.config.local if desired
ifndef DONT_BUILD_PARADYN
fullSystem	+= $(basicComps)
Build_list	+= basicComps
ifndef DONT_BUILD_FE
fullSystem	+= $(ParadynFE)
Build_list	+= ParadynFE
endif
ifndef DONT_BUILD_DAEMON
fullSystem	+= $(ParadynD)
Build_list	+= ParadynD
endif
ifndef DONT_BUILD_VISIS
fullSystem	+= $(ParadynVC)
Build_list	+= ParadynVC
endif
ifndef DONT_BUILD_PD_MT
fullSystem	+= $(threadComps)
Build_list	+= threadComps
endif
endif

ifndef DONT_BUILD_DYNINST
fullSystem	+= $(DyninstAPI)
Build_list	+= DyninstAPI
endif

# Note that the first rule listed ("all") is what gets made by default,
# i.e., if make is given no arguments.  Don't add other targets before all!

all: ready world

# This rule makes most of the normal recursive stuff.  Just about any
# target can be passed down to the lower-level Makefiles by listing it
# as a target here:

clean install depend distclean:
	+@for subsystem in $(fullSystem); do 			\
	    if [ -f $$subsystem/$(PLATFORM)/Makefile ]; then	\
		$(MAKE) -C $$subsystem/$(PLATFORM) $@;		\
	    else						\
		true;						\
	    fi							\
	done

# Check that the main installation directories, etc., are ready for a build,
# creating them if they don't already exist!

ready:
	+@for installdir in $(LIBRARY_DEST) $(PROGRAM_DEST); do \
	    if [ -d $$installdir ]; then			\
		echo "Installation directory $$installdir exists...";	\
	        true;						\
	    else						\
		echo "Creating installation directory $$installdir ...";\
	        mkdir -p $$installdir;				\
	    fi							\
	done

	@echo "Primary compiler for Paradyn build is:"
        @if [ $(CXX) = "xlC" ]; then \
               echo "xlC"; \
        else \
	  $(CXX) -v; \
        endif

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
ifdef DONT_BUILD_PARADYN
	@echo "Build of Paradyn components skipped!"
endif
ifdef DONT_BUILD_FE
	@echo "Build of Paradyn front-end components skipped!"
endif
ifdef DONT_BUILD_DAEMON
	@echo "Build of Paradyn daemon components skipped!"
endif
ifdef DONT_BUILD_VISIS
	@echo "Build of Paradyn visi client components skipped!"
endif
ifdef DONT_BUILD_PD_MT
	@echo "Build of ParadynMT components skipped!"
endif
ifdef DONT_BUILD_DYNINST
	@echo "Build of DyninstAPI components skipped!"
endif

world: intro
	+for target in $(Build_list); do				\
	    ( $(MAKE) $$target ) || exit 1;				\
	done
	@echo "Build of $(BUILD_ID) complete for $(PLATFORM)!"

# "make Paradyn" and "make DyninstAPI" are also useful and valid build targets!

Paradyn ParadynD ParadynFE ParadynVC DyninstAPI basicComps subSystems threadComps:
	@echo "Building $@ ..."
	@date
	+for subsystem in $($@); do				\
	    if [ -f $$subsystem/$(PLATFORM)/Makefile ]; then		\
		if ( $(MAKE) -C $$subsystem/$(PLATFORM) all ); then	\
		    $(MAKE) -C $$subsystem/$(PLATFORM) install;		\
		else							\
		    exit 1;						\
		fi							\
	    else							\
		true;							\
	    fi								\
	done
	@echo "Build of $@ complete."
	@date

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
	$(MAKE) DyninstAPI

nightly:
	$(MAKE) clean
	$(MAKE) world
#	$(MAKE) DyninstAPI
	$(MAKE) docs
	$(MAKE) install-man
	chmod 644 /p/paradyn/man/man?/*
