#
#
# First cut at a topLevel makefile for the paradyn system.
#
#

all:
	-cd util/$(PLATFORM); make install
	-cd igen/$(PLATFORM); make install
	-cd bininst/$(PLATFORM); make install
	-cd paradynd/$(PLATFORM); make install
	-cd paradyn/$(PLATFORM); make install
