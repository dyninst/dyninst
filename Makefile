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
buildfirst	= igen util thread visi dag hist
subsystems	= bininst paradyn paradynd paradyndSIM paradyndPVM \
		  rtinst rthist


# This rule makes most of the normal recursive stuff.  Just about any
# target can be passed down to the lower-level Makefiles by listing it
# as a target on the next line:

# Note that the first item listed in this rule ("all") is what gets
# made by default if make is passed no arguments.  Don't add other
# target before all!

all clean install:
	+for subsystem in $(buildfirst) $(subsystems); do 	\
	    if [ -f $$subsystem/$(PLATFORM)/Makefile ]; then	\
	       $(MAKE) -C $$subsystem/$(PLATFORM) $@;		\
	    else						\
		true;						\
	    fi							\
	done

# This rules passes down the documentation-related stuff to
# lower-level Makefiles in the individual "docs" directories.

docs install-man:
	+for subsystem in $(buildfirst) $(subsystems); do	\
	    if [ -f $$subsystem/docs/Makefile ]; then		\
	       $(MAKE) -C $$subsystem/docs $@;			\
	    else						\
		true;						\
	    fi							\
	done


# The "make world" target is set up to build things in the "correct"
# order for a build from scratch.  It builds things in the
# "buildfirst" list first, then builds everything, then installs
# everything.
#
# This make target doesn't go first in the Makefile, though, since we
# really only want to make "install" when it's specifically requested.
# Note that "world" doesn't do a "clean", since it's nice to be able
# to restart a compilation that fails without re-doing a lot of
# unnecessary work.

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
	$(MAKE) docs
	$(MAKE) install-man
	chmod 644 /p/paradyn/man/man?/*
