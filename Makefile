#
#
# First cut at a topLevel makefile for the paradyn system.
#
#

# Standalone is the list of standalone binaries.  These get built
# first when we "make world", since they include tools for building
# the rest of the system.
#
# Subsystems is the list of all other pieces which should be built.
#
buildfirst	= igen util thread
subsystems	= igen bininst util rtinst thread paradyn paradynd paradyndSIM


# This rule makes most of the normal recursive stuff.  Just about any
# target can be passed down to the lower-level Makefiles by listing it
# as a target on the next line:

# Note that the first item listed in this rule ("all") is what gets
# made by default if make is passed no arguments.  Don't add other
# target before all!

all clean install:
	+for subsystem in $(subsystems); do 			\
	    if [ -f $$subsystem/$(PLATFORM)/Makefile ]; then	\
	       $(MAKE) -C $$subsystem/$(PLATFORM) $@;		\
	    else						\
		true;						\
	    fi							\
	done

# This rules passes down the documentation-related stuff to
# lower-level Makefiles in the individual "docs" directories.

docs install-man:
	+for subsystem in $(subsystems); do 			\
	    if [ -f $$subsystem/docs/Makefile ]; then	\
	       $(MAKE) -C $$subsystem/docs $@;			\
	    else						\
		true;						\
	    fi							\
	done


# The "make world" target is what should get run automatically every
# night.  It builds things in the right order for a build from
# scratch.  This doesn't go first in the Makefile, though, since we
# really only want to make "install" when it's specifically requested.
# Note that "world" doesn't do a "clean", since it's nice to be able
# to restart a compilation that fails without re-doing a lot of work.
# Instead, the nightly build should be a "make clean world".

world:
	+for subsystem in $(buildfirst); do 			\
	    if [ -f $$subsystem/$(PLATFORM)/Makefile ]; then	\
	       $(MAKE) -C $$subsystem/$(PLATFORM) all install;	\
	    else						\
		true;						\
	    fi							\
	done
	$(MAKE) all
	$(MAKE) install
	$(MAKE) docs
	$(MAKE) install-man
	chmod 644 /var/home/paradyn/man/man?/*
