
include	../../make.config

C_OBJS1	= test1.o client1.o test1.CLNT.o 
S_OBJS1	= test1.o server1.o test1.SRVR.o 
OBJS2	= test2.o  client2.o server2.o 

all: client1 server1 program2

client1: $(C_OBJS1) 
	$(CC) $(CFLAGS) -o client1 $(C_OBJS1) $(LIBDIR) -lutil

server1: $(S_OBJS1) 
	$(CC) $(CFLAGS) -o server1 $(S_OBJS1) $(LIBDIR) -lutil

program2: $(OBJS2) 
	$(CC) $(CFLAGS) -o program2 $(OBJS2) $(LIBDIR) -lthread -lutil

clean:
	rm *.o *.CLNT.C *.SRVR.C *.h client1 server1 program2 test1.C test2.C

.SUFFIXES: .c .I .C .o

test1.C: test1.I ../$(PLATFORM)/igen
	../$(PLATFORM)/igen -xdr $<

test2.C: test2.I ../$(PLATFORM)/igen
	../$(PLATFORM)/igen -thread $<
