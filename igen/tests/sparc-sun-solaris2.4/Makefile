
include ../../../make.config



include ../Makefile.base
VPATH += ..
LIBS += -lsocket -lnsl
IFLAGS += -I/usr/psup/gcc/lib/g++-include -I../../..
CFLAGS += -Dsparc_sun_solaris2_3 $(IFLAGS)
CXXFLAGS +=  -Dsparc_sun_solaris2_3 $(IFLAGS)  -DDO_INLINE="inline"
TFLAGS +=  -Dsparc_sun_solaris2_3 $(IFLAGS)  -DDO_INLINE="inline"
CXX = g++
CC = gcc
