
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
	         $(XDRC_PVM) $(LIBDIR) -lutil -lpvm3 

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
	            $(C_OBJSPVM) $(LIBDIR) -lutil -lpvm3 

serverPVM_pure: $(S_OBJSPVM) 
	purify -g++ -collector=/usr/sup/purify/collect2 $(CC) -o serverPVM \
	         $(S_OBJSPVM) $(LIBDIR) -lutil -lpvm3 


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
	$(CC) $(CFLAGS) -o clientPVM $(C_OBJSPVM) $(LIBDIR) -lutil -lpvm3 

serverPVM: $(S_OBJSPVM) 
	$(CC) $(CFLAGS) -o serverPVM $(S_OBJSPVM) $(LIBDIR) -lutil -lpvm3 

server_xdr: $(S_OBJSXDR) 
	$(CC) $(CFLAGS) -o server_xdr $(S_OBJSXDR) $(LIBDIR) -lutil

xdrc_pvm: $(XDRC_PVM)
	$(CC) $(CFLAGS) -o xdrc_pvm $(XDRC_PVM) $(LIBDIR) -lutil -lpvm3 

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

client1.o: /usr/include/unistd.h /usr/include/assert.h test1.h
client1.o: ../../util/h/rpcUtil.h /usr/include/rpc/types.h
client1.o: /usr/include/malloc.h /usr/include/sys/types.h
client1.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h
client1.o: /usr/include/sys/time.h /usr/include/sys/time.h
client1.o: /usr/include/rpc/xdr.h /usr/include/sys/socket.h
client1.o: /usr/include/netinet/in.h /usr/include/netdb.h
client1.o: /usr/include/sys/file.h /usr/include/sys/fcntlcom.h
client1.o: /usr/include/sys/stat.h
client2.o: /usr/include/unistd.h /usr/include/assert.h test2.h
client2.o: ../../util/h/rpcUtil.h /usr/include/rpc/types.h
client2.o: /usr/include/malloc.h /usr/include/sys/types.h
client2.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h
client2.o: /usr/include/sys/time.h /usr/include/sys/time.h
client2.o: /usr/include/rpc/xdr.h /usr/include/sys/socket.h
client2.o: /usr/include/netinet/in.h /usr/include/netdb.h
client2.o: /usr/include/sys/file.h /usr/include/sys/fcntlcom.h
client2.o: /usr/include/sys/stat.h
client3.o: /usr/include/rpc/xdr.h /usr/include/rpc/types.h
client3.o: /usr/include/malloc.h /usr/include/sys/types.h
client3.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h
client3.o: /usr/include/sys/time.h /usr/include/sys/time.h
client3.o: /usr/include/unistd.h /usr/include/assert.h test3.CLNT.h test3.h
client3.o: ../../util/h/rpcUtil.h /usr/include/sys/socket.h
client3.o: /usr/include/netinet/in.h /usr/include/netdb.h
client3.o: /usr/include/sys/file.h /usr/include/sys/fcntlcom.h
client3.o: /usr/include/sys/stat.h /usr/include/string.h
client4.o: /usr/include/rpc/xdr.h /usr/include/rpc/types.h
client4.o: /usr/include/malloc.h /usr/include/sys/types.h
client4.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h
client4.o: /usr/include/sys/time.h /usr/include/sys/time.h
client4.o: /usr/include/unistd.h /usr/include/assert.h test4.CLNT.h test4.h
client4.o: ../../util/h/rpcUtil.h /usr/include/sys/socket.h
client4.o: /usr/include/netinet/in.h /usr/include/netdb.h
client4.o: /usr/include/sys/file.h /usr/include/sys/fcntlcom.h
client4.o: /usr/include/sys/stat.h
server1.o: /usr/include/stdio.h /usr/include/stdlib.h
server1.o: /usr/include/sys/stdtypes.h test1.h ../../util/h/rpcUtil.h
server1.o: /usr/include/rpc/types.h /usr/include/malloc.h
server1.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h
server1.o: /usr/include/sys/time.h /usr/include/sys/time.h
server1.o: /usr/include/rpc/xdr.h /usr/include/sys/socket.h
server1.o: /usr/include/netinet/in.h /usr/include/netdb.h
server1.o: /usr/include/unistd.h /usr/include/sys/file.h
server1.o: /usr/include/sys/fcntlcom.h /usr/include/sys/stat.h
server2.o: /usr/include/stdio.h /usr/include/stdlib.h
server2.o: /usr/include/sys/stdtypes.h test2.h ../../util/h/rpcUtil.h
server2.o: /usr/include/rpc/types.h /usr/include/malloc.h
server2.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h
server2.o: /usr/include/sys/time.h /usr/include/sys/time.h
server2.o: /usr/include/rpc/xdr.h /usr/include/sys/socket.h
server2.o: /usr/include/netinet/in.h /usr/include/netdb.h
server2.o: /usr/include/unistd.h /usr/include/sys/file.h
server2.o: /usr/include/sys/fcntlcom.h /usr/include/sys/stat.h
server3.o: /usr/include/stdio.h /usr/include/stdlib.h
server3.o: /usr/include/sys/stdtypes.h test3.SRVR.h test3.h
server3.o: ../../util/h/rpcUtil.h /usr/include/rpc/types.h
server3.o: /usr/include/malloc.h /usr/include/sys/types.h
server3.o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
server3.o: /usr/include/sys/time.h /usr/include/rpc/xdr.h
server3.o: /usr/include/sys/socket.h /usr/include/netinet/in.h
server3.o: /usr/include/netdb.h /usr/include/unistd.h /usr/include/sys/file.h
server3.o: /usr/include/sys/fcntlcom.h /usr/include/sys/stat.h
server3.o: /usr/include/string.h
server4.o: /usr/include/stdio.h /usr/include/stdlib.h
server4.o: /usr/include/sys/stdtypes.h test4.SRVR.h test4.h
server4.o: ../../util/h/rpcUtil.h /usr/include/rpc/types.h
server4.o: /usr/include/malloc.h /usr/include/sys/types.h
server4.o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
server4.o: /usr/include/sys/time.h /usr/include/rpc/xdr.h
server4.o: /usr/include/sys/socket.h /usr/include/netinet/in.h
server4.o: /usr/include/netdb.h /usr/include/unistd.h /usr/include/sys/file.h
server4.o: /usr/include/sys/fcntlcom.h /usr/include/sys/stat.h
clientPVM.o: /usr/include/rpc/xdr.h /usr/include/rpc/types.h
clientPVM.o: /usr/include/malloc.h /usr/include/sys/types.h
clientPVM.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h
clientPVM.o: /usr/include/sys/time.h /usr/include/sys/time.h
clientPVM.o: /usr/include/unistd.h /usr/include/assert.h testPVM.CLNT.h
clientPVM.o: testPVM.h ../../util/h/rpcUtil.h /usr/include/sys/socket.h
clientPVM.o: /usr/include/netinet/in.h /usr/include/netdb.h
clientPVM.o: /usr/include/sys/file.h /usr/include/sys/fcntlcom.h
clientPVM.o: /usr/include/sys/stat.h ../../util/h/rpcUtilPVM.h
clientPVM.o: /paradyn/packages/pvm3-dyn/include/pvm3.h
serverPVM.o: /usr/include/stdio.h /usr/include/stdlib.h
serverPVM.o: /usr/include/sys/stdtypes.h testPVM.SRVR.h testPVM.h
serverPVM.o: ../../util/h/rpcUtil.h /usr/include/rpc/types.h
serverPVM.o: /usr/include/malloc.h /usr/include/sys/types.h
serverPVM.o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
serverPVM.o: /usr/include/sys/time.h /usr/include/rpc/xdr.h
serverPVM.o: /usr/include/sys/socket.h /usr/include/netinet/in.h
serverPVM.o: /usr/include/netdb.h /usr/include/unistd.h
serverPVM.o: /usr/include/sys/file.h /usr/include/sys/fcntlcom.h
serverPVM.o: /usr/include/sys/stat.h ../../util/h/rpcUtilPVM.h
serverPVM.o: /paradyn/packages/pvm3-dyn/include/pvm3.h
xdrc_pvm.o: /usr/include/stdio.h xdr_link.CLNT.h xdr_link.h
xdrc_pvm.o: ../../util/h/rpcUtil.h /usr/include/rpc/types.h
xdrc_pvm.o: /usr/include/malloc.h /usr/include/sys/types.h
xdrc_pvm.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h
xdrc_pvm.o: /usr/include/sys/time.h /usr/include/sys/time.h
xdrc_pvm.o: /usr/include/rpc/xdr.h /usr/include/sys/socket.h
xdrc_pvm.o: /usr/include/netinet/in.h /usr/include/netdb.h
xdrc_pvm.o: /usr/include/unistd.h /usr/include/sys/file.h
xdrc_pvm.o: /usr/include/sys/fcntlcom.h /usr/include/sys/stat.h
xdrc_pvm.o: ../../util/h/list.h /usr/include/stdlib.h
xdrc_pvm.o: /paradyn/packages/pvm3-dyn/include/pvm3.h /usr/include/string.h
xdrc_pvm.o: /usr/include/assert.h
server_xdr.o: /usr/include/stdio.h /usr/include/stdlib.h
server_xdr.o: /usr/include/sys/stdtypes.h xdr_link.SRVR.h xdr_link.h
server_xdr.o: ../../util/h/rpcUtil.h /usr/include/rpc/types.h
server_xdr.o: /usr/include/malloc.h /usr/include/sys/types.h
server_xdr.o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
server_xdr.o: /usr/include/sys/time.h /usr/include/rpc/xdr.h
server_xdr.o: /usr/include/sys/socket.h /usr/include/netinet/in.h
server_xdr.o: /usr/include/netdb.h /usr/include/unistd.h
server_xdr.o: /usr/include/sys/file.h /usr/include/sys/fcntlcom.h
server_xdr.o: /usr/include/sys/stat.h ../../util/h/list.h
server_xdr.o: /usr/include/signal.h /usr/include/sys/signal.h
server_xdr.o: /usr/include/vm/faultcode.h
