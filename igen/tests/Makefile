
USES_PVM = true

include	../../make.config

.SUFFIXES: .c .I .C .o

C_OBJS3	= test3.o client3.o test3.CLNT.o 
S_OBJS3	= test3.o server3.o test3.SRVR.o 
OBJS4	= test4.o client4.o server4.o 
C_OBJSPVM	= testPVM.o clientPVM.o testPVM.CLNT.o 
S_OBJSPVM	= testPVM.o serverPVM.o testPVM.SRVR.o 
S_OBJSXDR	= xdr_link.o server_xdr.o xdr_link.SRVR.o
XDRC_PVM	= xdr_link.o xdrc_pvm.o xdr_link.CLNT.o 
C_OBJS1	= test1.o client1.o test1.CLNT.o 
S_OBJS1	= test1.o server1.o test1.SRVR.o 
OBJS2	= test2.o client2.o server2.o 

SRCS 	= client1.C \
          client2.C \
          client3.C \
          client4.C \
          server1.C \
          server2.C \
          server3.C \
          server4.C \
          clientPVM.C \
          serverPVM.C \
          xdrc_pvm.C \
          server_xdr.C
           
EXTRA_CFLAGS = $(CFLAGS) -I./util/h

all:  client3 server3 program4 clientPVM serverPVM xdrc_pvm server_xdr \
      client1 server1 program2

all_igen: test1_igen test2_igen test3_igen test4_igen testPVM_igen xdr_link_igen

all_pure: client3_pure server3_pure program4_pure clientPVM_pure \
          serverPVM_pure server_xdr_pure xdrc_pvm_pure

xdrc_pvm_pure: $(XDRC_PVM)
	purify -g++ -collector=/usr/sup/purify/collect2 $(CC) -o xdrc_pvm \
	         $(XDRC_PVM) $(LIBDIR) -lutil -lutilPVM -lpvm3 

server_xdr_pure: $(S_OBJSXDR)
	purify -g++ -collector=/usr/sup/purify/collect2 $(CC) -o server_xdr \
	         $(S_OBJSXDR) $(LIBDIR) -lutil

client3_pure: $(C_OBJS3) 
	purify -g++ -collector=/usr/sup/purify/collect2 $(CC) -o client3 \
	         $(C_OBJS3) $(LIBDIR) -lutil

server3_pure: $(S_OBJS3) 
	purify -g++ -collector=/usr/sup/purify/collect2 $(CC) -o server3  \
	         $(S_OBJS3) $(LIBDIR) -lutil

client1_pure: $(C_OBJS3) 
	purify -g++ -collector=/usr/sup/purify/collect2 $(CC) -o client1 \
	         $(C_OBJS1) $(LIBDIR) -lutil

server1_pure: $(S_OBJS3) 
	purify -g++ -collector=/usr/sup/purify/collect2 $(CC) -o server1  \
	         $(S_OBJS1) $(LIBDIR) -lutil

program2_pure: $(OBJS4) 
	purify -g++ -collector=/usr/sup/purify/collect2 $(CC) -o program2 \
	          $(OBJS2) $(LIBDIR) -lthread -lutil

program4_pure: $(OBJS4) 
	purify -g++ -collector=/usr/sup/purify/collect2 $(CC) -o program4 \
	          $(OBJS4) $(LIBDIR) -lthread -lutil

clientPVM_pure: $(C_OBJSPVM) 
	purify -g++ -collector=/usr/sup/purify/collect2 $(CC) -o clientPVM \
	            $(C_OBJSPVM) $(LIBDIR) -lutil -lutilPVM -lpvm3 

serverPVM_pure: $(S_OBJSPVM) 
	purify -g++ -collector=/usr/sup/purify/collect2 $(CC) -o serverPVM \
	         $(S_OBJSPVM) $(LIBDIR) -lutil -lutilPVM -lpvm3 


client3: $(C_OBJS3) 
	$(CC) $(CFLAGS) -o client3 $(C_OBJS3) $(LIBDIR) -lutil

server3: $(S_OBJS3) 
	$(CC) $(CFLAGS) -o server3 $(S_OBJS3) $(LIBDIR) -lutil

client1: $(C_OBJS1) 
	$(CC) $(CFLAGS) -o client1 $(C_OBJS1) $(LIBDIR) -lutil

server1: $(S_OBJS1) 
	$(CC) $(CFLAGS) -o server1 $(S_OBJS1) $(LIBDIR) -lutil

program2: $(OBJS2) 
	$(CC) $(CFLAGS) -o program2 $(OBJS2) $(LIBDIR) -lthread -lutil

program4: $(OBJS4) 
	$(CC) $(CFLAGS) -o program4 $(OBJS4) $(LIBDIR) -lthread -lutil

clientPVM: $(C_OBJSPVM) 
	$(CC) $(CFLAGS) -o clientPVM $(C_OBJSPVM) $(LIBDIR) -lutil -lutilPVM -lpvm3 

serverPVM: $(S_OBJSPVM) 
	$(CC) $(CFLAGS) -o serverPVM $(S_OBJSPVM) $(LIBDIR) -lutil -lutilPVM -lpvm3 

server_xdr: $(S_OBJSXDR) 
	$(CC) $(CFLAGS) -o server_xdr $(S_OBJSXDR) $(LIBDIR) -lutil

xdrc_pvm: $(XDRC_PVM)
	$(CC) $(CFLAGS) -o xdrc_pvm $(XDRC_PVM) $(LIBDIR) -lutil -lutilPVM -lpvm3 

clean: 
	rm *.o *.CLNT.C *.SRVR.C *.h client3 server3 program4 test3.C \
              test4.C xdrc_pvm server_xdr clientPVM serverPVM \
              xdr_link.C  testPVM.C *pure* client1 server1 program2 \
              test1.C test2.C

test1_igen: test1.I
	../$(PLATFORM)/igen -xdr $<

test2_igen: test2.I
	../$(PLATFORM)/igen -thread $<

test3_igen: test3.I
	../$(PLATFORM)/igen -xdr $<

test4_igen: test4.I
	../$(PLATFORM)/igen -thread $<

testPVM_igen: testPVM.I
	../$(PLATFORM)/igen -pvm $<

xdr_link_igen: xdr_link.I
	../$(PLATFORM)/igen -xdr $<

test2.C: test2.I ../$(PLATFORM)/igen
	../$(PLATFORM)/igen -thread $<

test1.C: test1.I ../$(PLATFORM)/igen
	../$(PLATFORM)/igen -xdr $<

test1.h: test1.I ../$(PLATFORM)/igen
	../$(PLATFORM)/igen -xdr $<

test3.C: test3.I ../$(PLATFORM)/igen
	../$(PLATFORM)/igen -xdr $<

test4.C: test4.I ../$(PLATFORM)/igen
	../$(PLATFORM)/igen -thread $<

testPVM.C: testPVM.I ../$(PLATFORM)/igen
	../$(PLATFORM)/igen -pvm $<

xdr_link.C: xdr_link.I ../$(PLATFORM)/igen
	../$(PLATFORM)/igen -xdr $<

depend: $(SRCS)
	makedepend $(SRCS) $(CFLAGS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

client1.o: /usr/include/unistd.h /usr/include/assert.h
client2.o: /usr/include/unistd.h /usr/include/assert.h
client3.o: /usr/include/rpc/xdr.h /usr/include/rpc/types.h
client3.o: /usr/include/malloc.h /usr/include/sys/types.h
client3.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h
client3.o: /usr/include/sys/time.h /usr/include/sys/time.h
client3.o: /usr/include/unistd.h /usr/include/assert.h /usr/include/string.h
client4.o: /usr/include/rpc/xdr.h /usr/include/rpc/types.h
client4.o: /usr/include/malloc.h /usr/include/sys/types.h
client4.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h
client4.o: /usr/include/sys/time.h /usr/include/sys/time.h
client4.o: /usr/include/unistd.h /usr/include/assert.h
server1.o: /usr/include/stdio.h /usr/include/stdlib.h
server1.o: /usr/include/sys/stdtypes.h
server2.o: /usr/include/stdio.h /usr/include/stdlib.h
server2.o: /usr/include/sys/stdtypes.h
server3.o: /usr/include/stdio.h /usr/include/stdlib.h
server3.o: /usr/include/sys/stdtypes.h /usr/include/string.h
server4.o: /usr/include/stdio.h /usr/include/stdlib.h
server4.o: /usr/include/sys/stdtypes.h
clientPVM.o: /usr/include/rpc/xdr.h /usr/include/rpc/types.h
clientPVM.o: /usr/include/malloc.h /usr/include/sys/types.h
clientPVM.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h
clientPVM.o: /usr/include/sys/time.h /usr/include/sys/time.h
clientPVM.o: /usr/include/unistd.h /usr/include/assert.h
serverPVM.o: /usr/include/stdio.h /usr/include/stdlib.h
serverPVM.o: /usr/include/sys/stdtypes.h
xdrc_pvm.o: /usr/include/stdio.h ../../util/h/list.h /usr/include/stdlib.h
xdrc_pvm.o: /usr/include/sys/stdtypes.h
xdrc_pvm.o: /paradyn/packages/pvm3-dyn/include/pvm3.h /usr/include/string.h
xdrc_pvm.o: /usr/include/assert.h
server_xdr.o: /usr/include/stdio.h /usr/include/stdlib.h
server_xdr.o: /usr/include/sys/stdtypes.h ../../util/h/list.h
server_xdr.o: /usr/include/signal.h /usr/include/sys/signal.h
server_xdr.o: /usr/include/vm/faultcode.h
