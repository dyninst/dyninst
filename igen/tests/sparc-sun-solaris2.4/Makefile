
include ../../../make.config



include ../Makefile.base
VPATH += ..
LIBS += -lsocket -lnsl
IFLAGS += -I/usr/psup/gcc/lib/g++-include -I../../..
CFLAGS += -Dsparc_sun_solaris2_4 $(IFLAGS)
CXXFLAGS =  -Dsparc_sun_solaris2_4 $(IFLAGS)  -DDO_INLINE_P="inline" -DDO_INLINE_F="inline"
#TFLAGS +=  -Dsparc_sun_solaris2_4 $(IFLAGS)  -DDO_INLINE="inline"
CXX	= $(GXX)
CC	= $(GCC)
