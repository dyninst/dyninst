#
#
# First cut at a topLevel makefile for the paradyn system.
#
#


subsystems	= util rtinst thread paradyn paradynd paradyndSIM igen bininst

all clean install:
	for subsystem in $(subsystems); do 			\
	    if [ -f $$subsystem/$(PLATFORM)/Makefile ]; then	\
	       $(MAKE) -C $$subsystem/$(PLATFORM) $@;		\
	    else						\
		true;						\
	    fi							\
	done

man install-man:
	for subsystem in $(subsystems); do 			\
	    if [ -f $$subsystem/$(PLATFORM)/Makefile ]; then	\
	       $(MAKE) -C $$subsystem/docs $@;			\
	    else						\
		true;						\
	    fi							\
	done
