SHELL = /bin/sh
.SUFFIXES: .C .o

CC = g++
VPATH = ../src:/usr/openwin/lib
IFLAGS = -I. -I/p/paradyn/development/newhall/core/visi/h 
IFLAGS += -I/usr/openwin/include
LIBDIR = -L/p/paradyn/development/newhall/lib/sparc-sun-solaris2.4 
LIBDIR += -R/usr/openwin/lib -L/usr/openwin/lib
CFLAGS = -O -g $(IFLAGS) 


LIBS = $(LIBDIR) -lvisi -lpdutilOld -lstdc++ -lXaw -lXpm -lXext -lXmu -lXt
LIBS += -lX11 -lm -lsocket -lnsl

SRCS = ../src/xtext.C
OBJS = xtext.o

all: xtext 

xtext: $(OBJS)
	$(CC) -o xtext $(LFLAGS) $(OBJS) $(LIBS) 

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) -c $(SRCS) 

clean:
	rm -f $(OBJS) xtext
