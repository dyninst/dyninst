#
#
# First cut at a topLevel makefile for the paradyn system.
#
#


subsystems	= util rtinst thread paradyn paradynd paradyndSIM igen bininst

all clean install:
	for subsystem in $(subsystems); do 			\
	    if [ -f $$subsystem/$(PLATFORM)/Makefile ]; then	\
	       cd $$subsystem/$(PLATFORM); $(MAKE) $@; cd ../..	\
	    else true; fi					\
	done

man install-man:
	for subsystem in $(subsystems); do 			\
	    if [ -f $$subsystem/docs/Makefile ]; then	\
	       cd $$subsystem/docs; $(MAKE) $@; cd ../..	\
	    else true; fi					\
	done
