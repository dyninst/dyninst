
include 	../../make.config

CFLAGS		+=  -I./util/h -I../sparc-sun-sunos4.1.3
CC		= /usr/psup/bin/g++
AS		= /bin/as

VPATH	      = ../src
LIBRARY	      = visilib.a

SRVROBJ       = server1.o

CLNTOBJS      = client1.o

OBJS	      = client1.o server1.o xtext.o xtext2.o

SRCS	      = client1.C server1.C xtext.C xtext.C

PRINT	      = pr

all: 	client1 server1 xtext xtext2 



server1:	$(SRVROBJ)
	/usr/sup/purify/purify -g++ -collector=/usr/sup/purify/collect2 $(CC) -static $(CFLAGS) $(LIBDIR) -o server1 $(SRVROBJ) -lm $(LIBDIR) -lutil -lvisi

client1:	$(CLNTOBJS)
	/usr/sup/purify/purify -g++ -collector=/usr/sup/purify/collect2 $(CC) -static $(CFLAGS) $(LIBDIR) -o client1 $(CLNTOBJS) -lm $(LIBDIR) -lutil -lvisi

xtext: xtext.o
	$(CC) xtext.o -o xtext -lXaw -lXmu -lXt -lX11 -lm $(LIBDIR) -lutil -lvisi

xtext2: xtext2.o
	$(CC) xtext2.o -o xtext2 -lXaw -lXmu -lXt -lX11 -lm $(LIBDIR) -lutil -lvisi


clean:	
	rm -f $(OBJS) server1 client1 xtext xtext2 *pure*

depend: $(SRCS)
	@echo "Building dependancies"
	/usr/sup/X11/bin/makedepend $(SRCS) $(CFLAGS)

library:        $(LIBRARY)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

update:         $(DEST)/$(LIBRARY)

# DO NOT DELETE THIS LINE -- make depend depends on it.

client1.o: ../h/visithread.h ../sparc-sun-sunos4.1.3/visi.CLNT.h
client1.o: ../sparc-sun-sunos4.1.3/visi.h ../../util/h/rpcUtil.h
client1.o: /usr/include/stdio.h /usr/include/strings.h /usr/include/string.h
client1.o: /usr/include/sys/stdtypes.h /usr/include/stdlib.h
client1.o: /usr/include/errno.h /usr/include/sys/errno.h
client1.o: /usr/include/assert.h /usr/include/fcntl.h
client1.o: /usr/include/sys/fcntlcom.h /usr/include/sys/stat.h
client1.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h
client1.o: /usr/include/memory.h /usr/include/sys/socket.h
client1.o: /usr/include/netinet/in.h /usr/include/netdb.h
client1.o: /usr/include/sys/time.h /usr/include/sys/time.h
client1.o: /usr/include/unistd.h /usr/include/sys/file.h
client1.o: /usr/include/rpc/types.h /usr/include/malloc.h
client1.o: /usr/include/rpc/xdr.h
server1.o: ../h/visualization.h ../h/datagrid.h /usr/include/string.h
server1.o: /usr/include/sys/stdtypes.h /usr/include/math.h
server1.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
server1.o: /usr/include/values.h ../h/error.h /usr/include/stdio.h
server1.o: ../sparc-sun-sunos4.1.3/visi.h ../../util/h/rpcUtil.h
server1.o: /usr/include/strings.h /usr/include/stdlib.h /usr/include/errno.h
server1.o: /usr/include/sys/errno.h /usr/include/assert.h
server1.o: /usr/include/fcntl.h /usr/include/sys/fcntlcom.h
server1.o: /usr/include/sys/stat.h /usr/include/sys/types.h
server1.o: /usr/include/sys/sysmacros.h /usr/include/memory.h
server1.o: /usr/include/sys/socket.h /usr/include/netinet/in.h
server1.o: /usr/include/netdb.h /usr/include/sys/time.h
server1.o: /usr/include/sys/time.h /usr/include/unistd.h
server1.o: /usr/include/sys/file.h /usr/include/rpc/types.h
server1.o: /usr/include/malloc.h /usr/include/rpc/xdr.h ../h/mrlist.h
server1.o: ../sparc-sun-sunos4.1.3/visi.SRVR.h
xtext.o: /usr/include/stdio.h /usr/include/X11/Intrinsic.h
xtext.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
xtext.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h
xtext.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
xtext.o: /usr/include/X11/Xosdefs.h /usr/include/stddef.h
xtext.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
xtext.o: /usr/include/string.h /usr/include/X11/Core.h
xtext.o: /usr/include/X11/Composite.h /usr/include/X11/Constraint.h
xtext.o: /usr/include/X11/Object.h /usr/include/X11/RectObj.h
xtext.o: /usr/include/X11/StringDefs.h /usr/include/X11/Xaw/AsciiText.h
xtext.o: /usr/include/X11/Xaw/Text.h /usr/include/X11/Xaw/TextSink.h
xtext.o: /usr/include/X11/Xaw/TextSrc.h /usr/include/X11/Xaw/AsciiSrc.h
xtext.o: /usr/include/X11/Xaw/AsciiSink.h /usr/include/X11/Xaw/Command.h
xtext.o: /usr/include/X11/Xaw/Label.h /usr/include/X11/Xaw/Simple.h
xtext.o: /usr/include/X11/Xmu/Converters.h /usr/include/X11/Xaw/Paned.h
xtext.o: /usr/include/X11/Xaw/Cardinals.h ../h/visualization.h
xtext.o: ../h/datagrid.h /usr/include/math.h /usr/include/floatingpoint.h
xtext.o: /usr/include/sys/ieeefp.h /usr/include/values.h ../h/error.h
xtext.o: ../sparc-sun-sunos4.1.3/visi.h ../../util/h/rpcUtil.h
xtext.o: /usr/include/strings.h /usr/include/stdlib.h /usr/include/errno.h
xtext.o: /usr/include/sys/errno.h /usr/include/assert.h /usr/include/fcntl.h
xtext.o: /usr/include/sys/fcntlcom.h /usr/include/sys/stat.h
xtext.o: /usr/include/memory.h /usr/include/sys/socket.h
xtext.o: /usr/include/netinet/in.h /usr/include/netdb.h
xtext.o: /usr/include/sys/time.h /usr/include/sys/time.h
xtext.o: /usr/include/unistd.h /usr/include/sys/file.h
xtext.o: /usr/include/rpc/types.h /usr/include/malloc.h
xtext.o: /usr/include/rpc/xdr.h ../h/mrlist.h
xtext.o: ../sparc-sun-sunos4.1.3/visi.SRVR.h
xtext.o: /usr/include/stdio.h /usr/include/X11/Intrinsic.h
xtext.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
xtext.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h
xtext.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
xtext.o: /usr/include/X11/Xosdefs.h /usr/include/stddef.h
xtext.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
xtext.o: /usr/include/string.h /usr/include/X11/Core.h
xtext.o: /usr/include/X11/Composite.h /usr/include/X11/Constraint.h
xtext.o: /usr/include/X11/Object.h /usr/include/X11/RectObj.h
xtext.o: /usr/include/X11/StringDefs.h /usr/include/X11/Xaw/AsciiText.h
xtext.o: /usr/include/X11/Xaw/Text.h /usr/include/X11/Xaw/TextSink.h
xtext.o: /usr/include/X11/Xaw/TextSrc.h /usr/include/X11/Xaw/AsciiSrc.h
xtext.o: /usr/include/X11/Xaw/AsciiSink.h /usr/include/X11/Xaw/Command.h
xtext.o: /usr/include/X11/Xaw/Label.h /usr/include/X11/Xaw/Simple.h
xtext.o: /usr/include/X11/Xmu/Converters.h /usr/include/X11/Xaw/Paned.h
xtext.o: /usr/include/X11/Xaw/Cardinals.h ../h/visualization.h
xtext.o: ../h/datagrid.h /usr/include/math.h /usr/include/floatingpoint.h
xtext.o: /usr/include/sys/ieeefp.h /usr/include/values.h ../h/error.h
xtext.o: ../sparc-sun-sunos4.1.3/visi.h ../../util/h/rpcUtil.h
xtext.o: /usr/include/strings.h /usr/include/stdlib.h /usr/include/errno.h
xtext.o: /usr/include/sys/errno.h /usr/include/assert.h /usr/include/fcntl.h
xtext.o: /usr/include/sys/fcntlcom.h /usr/include/sys/stat.h
xtext.o: /usr/include/memory.h /usr/include/sys/socket.h
xtext.o: /usr/include/netinet/in.h /usr/include/netdb.h
xtext.o: /usr/include/sys/time.h /usr/include/sys/time.h
xtext.o: /usr/include/unistd.h /usr/include/sys/file.h
xtext.o: /usr/include/rpc/types.h /usr/include/malloc.h
xtext.o: /usr/include/rpc/xdr.h ../h/mrlist.h
xtext.o: ../sparc-sun-sunos4.1.3/visi.SRVR.h
