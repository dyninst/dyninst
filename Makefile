#
#
# First cut at a topLevel makefile for the paradyn system.
#
#


subsystems	= util rtinst thread paradyn paradynd paradyndSIM igen

all clean install install-man:
	for subsystem in $(subsystems); do 			\
	    if [ -f $$subsystem/$(PLATFORM)/Makefile ]; then	\
	       cd $$subsystem/$(PLATFORM); $(MAKE) $@; cd ../..	\
	    else true; fi					\
	done
