
include	../../make.config

C_OBJS1	= client1.o test1.o test1.CLNT.o 
S_OBJS1	= server1.o test1.o test1.SRVR.o 
OBJS2	= client2.o test2.o server2.o 

all: client1 server1 program2

client1: $(C_OBJS1) 
	$(CC) $(CFLAGS) -o client1 $(C_OBJS1) $(LIBDIR) -lutil

server1: $(S_OBJS1) 
	$(CC) $(CFLAGS) -o server1 $(S_OBJS1) $(LIBDIR) -lutil

program2: $(OBJS2) 
	$(CC) $(CFLAGS) -o program2 $(OBJS2) $(LIBDIR) -lthread -lutil

.SUFFIXES: .c .I .C .o

test1.C: test1.I ../$(PLATFORM)/igen
	../$(PLATFORM)/igen -xdr $<

test2.h: test2.I ../$(PLATFORM)/igen
	../$(PLATFORM)/igen -thread $<

test2.C: test2.h
test1.h: test1.h

