include 	../../make.config


VPATH	      = ../src
LIBRARY	      = visilib.a

SRVROBJ       = server1.o

CLNTOBJS      = client1.o visithread.o

CLNT2OBJS      = client2.o visithread.o

OBJS	      = client1.o server1.o xtext.o \
		xtext2.o client2.o visithread.o

SRCS	      = visi.C visi.CLNT.C visi.SRVR.C client1.C server1.C xtext.C \
		xtext2.C client2.C visithread.C

PRINT	      = pr

all: 	visi.C client1 server1 xtext xtext2 client2 



server1:	$(SRVROBJ)
	$(CC) -static $(CFLAGS) $(LIBDIR) -o server1 $(SRVROBJ) -lm $(LIBDIR) -lutil -lvisi

client1:	$(CLNTOBJS)
	$(CC) -static $(CFLAGS) $(LIBDIR) -o client1 $(CLNTOBJS) -lm $(LIBDIR) -lutil -lvisi

client2:	$(CLNT2OBJS)
	$(CC) -static $(CFLAGS) $(LIBDIR) -o client2 $(CLNT2OBJS) -lm $(LIBDIR) -lutil -lvisi

xtext: xtext.o
	$(CC) xtext.o -o xtext -lXaw -lXmu -lXt -lX11 -lm $(LIBDIR) -lutil -lvisi

xtext2: xtext2.o
	$(CC) xtext2.o -o xtext2 -lXaw -lXmu -lXt -lX11 -lm $(LIBDIR) -lutil -lvisi


clean:	
	rm -f $(OBJS) server1 client1 client2 xtext xtext2 *pure* visi.*

depend: visi.C $(SRCS)
	@echo "Building dependancies"
	/usr/sup/X11/bin/makedepend $(SRCS) $(CFLAGS)

visi.C visi.CLNT.C visi.SRVR.C: ../h/visi.I
	igen -xdr ../h/visi.I

library:        $(LIBRARY)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

update:         $(DEST)/$(LIBRARY)

# DO NOT DELETE THIS LINE -- make depend depends on it.

visi.o: /usr/include/stdio.h /usr/include/stdlib.h
visi.o: /usr/include/sys/stdtypes.h /usr/include/rpc/types.h
visi.o: /usr/include/malloc.h /usr/include/sys/types.h
visi.o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
visi.o: /usr/include/sys/time.h /usr/include/assert.h /usr/include/rpc/xdr.h
visi.o: /usr/include/errno.h /usr/include/sys/errno.h visi.h
visi.o: ../../util/h/rpcUtil.h /usr/include/unistd.h
visi.o: /usr/include/sys/socket.h
visi.CLNT.o: /usr/include/stdio.h /usr/include/stdlib.h
visi.CLNT.o: /usr/include/sys/stdtypes.h /usr/include/rpc/types.h
visi.CLNT.o: /usr/include/malloc.h /usr/include/sys/types.h
visi.CLNT.o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
visi.CLNT.o: /usr/include/sys/time.h /usr/include/assert.h
visi.CLNT.o: /usr/include/rpc/xdr.h /usr/include/errno.h
visi.CLNT.o: /usr/include/sys/errno.h visi.CLNT.h visi.h
visi.CLNT.o: ../../util/h/rpcUtil.h /usr/include/unistd.h
visi.CLNT.o: /usr/include/sys/socket.h
visi.SRVR.o: /usr/include/stdio.h /usr/include/stdlib.h
visi.SRVR.o: /usr/include/sys/stdtypes.h /usr/include/rpc/types.h
visi.SRVR.o: /usr/include/malloc.h /usr/include/sys/types.h
visi.SRVR.o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
visi.SRVR.o: /usr/include/sys/time.h /usr/include/assert.h
visi.SRVR.o: /usr/include/rpc/xdr.h /usr/include/errno.h
visi.SRVR.o: /usr/include/sys/errno.h visi.SRVR.h visi.h
visi.SRVR.o: ../../util/h/rpcUtil.h /usr/include/unistd.h
visi.SRVR.o: /usr/include/sys/socket.h
client1.o: /usr/include/stdio.h /usr/include/stdlib.h
client1.o: /usr/include/sys/stdtypes.h visi.CLNT.h visi.h
client1.o: ../../util/h/rpcUtil.h /usr/include/unistd.h
client1.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h
client1.o: /usr/include/sys/socket.h /usr/include/rpc/types.h
client1.o: /usr/include/malloc.h /usr/include/sys/time.h
client1.o: /usr/include/sys/time.h /usr/include/rpc/xdr.h
server1.o: ../h/visualization.h ../h/datagrid.h /usr/include/string.h
server1.o: /usr/include/sys/stdtypes.h /usr/include/math.h
server1.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
server1.o: /usr/include/values.h ../h/error.h /usr/include/stdio.h visi.h
server1.o: ../../util/h/rpcUtil.h /usr/include/unistd.h
server1.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h
server1.o: /usr/include/sys/socket.h /usr/include/rpc/types.h
server1.o: /usr/include/malloc.h /usr/include/sys/time.h
server1.o: /usr/include/sys/time.h /usr/include/rpc/xdr.h ../h/mrlist.h
server1.o: visi.SRVR.h
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
xtext.o: /usr/include/sys/ieeefp.h /usr/include/values.h ../h/error.h visi.h
xtext.o: ../../util/h/rpcUtil.h /usr/include/unistd.h
xtext.o: /usr/include/sys/socket.h /usr/include/rpc/types.h
xtext.o: /usr/include/malloc.h /usr/include/sys/time.h
xtext.o: /usr/include/sys/time.h /usr/include/rpc/xdr.h ../h/mrlist.h
xtext.o: visi.SRVR.h
xtext2.o: /usr/include/stdio.h /usr/include/X11/Intrinsic.h
xtext2.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
xtext2.o: /usr/include/sys/stdtypes.h /usr/include/sys/sysmacros.h
xtext2.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
xtext2.o: /usr/include/X11/Xosdefs.h /usr/include/stddef.h
xtext2.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
xtext2.o: /usr/include/string.h /usr/include/X11/Core.h
xtext2.o: /usr/include/X11/Composite.h /usr/include/X11/Constraint.h
xtext2.o: /usr/include/X11/Object.h /usr/include/X11/RectObj.h
xtext2.o: /usr/include/X11/StringDefs.h /usr/include/X11/Xaw/AsciiText.h
xtext2.o: /usr/include/X11/Xaw/Text.h /usr/include/X11/Xaw/TextSink.h
xtext2.o: /usr/include/X11/Xaw/TextSrc.h /usr/include/X11/Xaw/AsciiSrc.h
xtext2.o: /usr/include/X11/Xaw/AsciiSink.h /usr/include/X11/Xaw/Command.h
xtext2.o: /usr/include/X11/Xaw/Label.h /usr/include/X11/Xaw/Simple.h
xtext2.o: /usr/include/X11/Xmu/Converters.h /usr/include/X11/Xaw/Paned.h
xtext2.o: /usr/include/X11/Xaw/Cardinals.h ../h/visualization.h
xtext2.o: ../h/datagrid.h /usr/include/math.h /usr/include/floatingpoint.h
xtext2.o: /usr/include/sys/ieeefp.h /usr/include/values.h ../h/error.h visi.h
xtext2.o: ../../util/h/rpcUtil.h /usr/include/unistd.h
xtext2.o: /usr/include/sys/socket.h /usr/include/rpc/types.h
xtext2.o: /usr/include/malloc.h /usr/include/sys/time.h
xtext2.o: /usr/include/sys/time.h /usr/include/rpc/xdr.h ../h/mrlist.h
xtext2.o: visi.SRVR.h
client2.o: /usr/include/stdio.h /usr/include/stdlib.h
client2.o: /usr/include/sys/stdtypes.h visi.CLNT.h visi.h
client2.o: ../../util/h/rpcUtil.h /usr/include/unistd.h
client2.o: /usr/include/sys/types.h /usr/include/sys/sysmacros.h
client2.o: /usr/include/sys/socket.h /usr/include/rpc/types.h
client2.o: /usr/include/malloc.h /usr/include/sys/time.h
client2.o: /usr/include/sys/time.h /usr/include/rpc/xdr.h
visithread.o: visi.CLNT.h visi.h ../../util/h/rpcUtil.h /usr/include/unistd.h
visithread.o: /usr/include/sys/types.h /usr/include/sys/stdtypes.h
visithread.o: /usr/include/sys/sysmacros.h /usr/include/sys/socket.h
visithread.o: /usr/include/rpc/types.h /usr/include/malloc.h
visithread.o: /usr/include/sys/time.h /usr/include/sys/time.h
visithread.o: /usr/include/rpc/xdr.h /usr/include/stdio.h
