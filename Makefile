#
#
# First cut at a topLevel makefile for the paradyn system.
#
#


subsystems	= util rtinst thread paradyn paradynd igen

all:
	for subsystem in $(subsystems); do 			\
	    if [ -d $$subsystem/$(PLATFORM) ]; then 		\
	       cd $$subsystem/$(PLATFORM); make install; cd ../.. 	\
	    else true; fi					\
	done
