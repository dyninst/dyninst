#
# TopLevel Makefile for the Paradyn system.
#
# $Id: Makefile,v 1.26 1998/04/01 02:59:47 wylie Exp $
#

include ./make.config
-include ./make.config.local

BUILD_ID = "$(SUITE_NAME) v$(RELEASE_NUM)$(BUILD_MARK)$(BUILD_NUM)"

# "buildFirst" is the list of components which need to be built first
# with "make world", since they are used building the rest of the system.
#
# "subSystems" is the list of all other pieces which should be built
# as part of Paradyn.
#
# "dynInstAPI" is the list of additional API components (optional).
#
buildFirst	= util igen thread visi hist
subSystems	= paradyn rtinst rthist paradynd \
		visiClients/tclVisi visiClients/barchart \
		visiClients/tableVisi visiClients/phaseTable \
		visiClients/terrain
dynInstAPI	= dyninstAPI_RT dyninstAPI dyninstAPI/tests 

#
# "fullSystem" is the complete list of all components
#
fullSystem	= $(buildFirst) $(subSystems) $(dynInstAPI)

# This rule makes most of the normal recursive stuff.  Just about any
# target can be passed down to the lower-level Makefiles by listing it
# as a target on the first line:

# Note that the first item listed in this rule ("all") is what gets
# made by default if make is passed no arguments.  Don't add other
# targets before all!

all: ready world
	@echo "$(BUILD_ID) build complete for $(PLATFORM)!"

clean install depend:
	+@for subsystem in $(fullSystem); do 			\
	    if [ -f $$subsystem/$(PLATFORM)/Makefile ]; then	\
		$(MAKE) -C $$subsystem/$(PLATFORM) $@;		\
	    else						\
		true;						\
	    fi							\
	done

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
	@$(CXX) -v

# This rules passes down the documentation-related stuff to
# lower-level Makefiles in the individual "docs" directories.

docs install-man:
	+for subsystem in $(fullSystem); do			\
	    if [ -f $$subsystem/docs/Makefile ]; then		\
		$(MAKE) -C $$subsystem/docs $@;			\
	    else						\
		true;						\
	    fi							\
	done


# The "make world" target is set up to build things in the "correct"
# order for a build from scratch.  It builds and installs things in the
# "buildFirst" list first, then builds and installs the remaining
# Paradyn "subSystems" (excluding the currently optional dynInstAPI).
# NB: "make world" currently also builds the dynInstAPI components
# since the nightly build doesn't seem to use the "nightly" target.
#
# This make target doesn't go first in the Makefile, though, since we
# really only want to make "install" when it's specifically requested.
# Note that "world" doesn't do a "clean", since it's nice to be able
# to restart a compilation that fails without re-doing a lot of
# unnecessary work.

world:
	@echo "Making $(BUILD_ID) world for $(PLATFORM)!"
	@date
	+for subsystem in $(buildFirst); do 			\
	    if [ -f $$subsystem/$(PLATFORM)/Makefile ]; then	\
		$(MAKE) -C $$subsystem/$(PLATFORM) all install;	\
	    else						\
		true;						\
	    fi							\
	done
	+for subsystem in $(dynInstAPI); do			\
	    if [ -f $$subsystem/$(PLATFORM)/Makefile ]; then	\
		$(MAKE) -C $$subsystem/$(PLATFORM) all install;	\
	    else						\
		true;						\
	    fi							\
	done
	+for subsystem in $(subSystems); do 			\
	    if [ -f $$subsystem/$(PLATFORM)/Makefile ]; then	\
		$(MAKE) -C $$subsystem/$(PLATFORM) all install;	\
	    else						\
		true;						\
	    fi							\
	done
	@date

# Currently optional dynInstAPI build and install

dynInstAPI:
	+for subsystem in $(dynInstAPI); do			\
	    if [ -f $$subsystem/$(PLATFORM)/Makefile ]; then	\
		$(MAKE) -C $$subsystem/$(PLATFORM) all install;	\
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

nightly:
	$(MAKE) clean
	$(MAKE) world
#	$(MAKE) dynInstAPI
	$(MAKE) docs
	$(MAKE) install-man
	chmod 644 /p/paradyn/man/man?/*
