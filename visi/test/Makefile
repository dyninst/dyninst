IFLAGS          = -I. -I../.. 

SUFFIX          = .C:sC

VPATH	      = ../src

LIBDIR		= -L../../../lib/$(PLATFORM) -L/p/paradyn/lib/$(PLATFORM)

YFLAGS		= -d -y
YACC		= bison

.SUFFIXES: .I .c .C .o

DEST          = .

CFLAGS          = $(IFLAGS) -g -Wall

CC              = g++

.C.o:
	$(CC) -c $(CFLAGS) $<

LIBRARY	      = visilib.a

SRVROBJ       = server1.o

CLNTOBJS      = client1.o visithread.o

CLNT2OBJS      = client2.o visithread.o

OBJS	      = client1.o server1.o xtext.o \
		xtext2.o client2.o visithread.o

SRCS	      = visi.xdr.C visi.xdr.CLNT.C visi.xdr.SRVR.C \
		client1.C server1.C xtext.C xtext2.C \
		client2.C visithread.C

PRINT	      = pr

all: 	visi.C client1 server1 xtext xtext2 client2 



server1:	$(SRVROBJ)
	$(CC) -static $(CFLAGS) $(LIBDIR) -o server1 $(SRVROBJ) -lm $(LIBDIR) -lvisi -lutil

client1:	$(CLNTOBJS)
	$(CC) -static $(CFLAGS) $(LIBDIR) -o client1 $(CLNTOBJS) -lm $(LIBDIR) -lvisi -lutil

client2:	$(CLNT2OBJS)
	$(CC) -static $(CFLAGS) $(LIBDIR) -o client2 $(CLNT2OBJS) -lm $(LIBDIR) -lvisi -lutil

xtext: xtext.o
	$(CC) xtext.o -o xtext -lXaw -lXmu -lXt -lX11 -lm $(LIBDIR) -lvisi -lutil

xtext2: xtext2.o
	$(CC) xtext2.o -o xtext2 -lXaw -lXmu -lXt -lX11 -lm $(LIBDIR) -lvisi -lutil


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

visi.xdr.o: visi.xdr.h ../../util/h/rpcUtil.h ../../util/h/headers.h
visi.xdr.o: ../../util/h/String.h ../../util/h/Vector.h
visi.xdr.CLNT.o: visi.xdr.CLNT.h visi.xdr.h ../../util/h/rpcUtil.h
visi.xdr.CLNT.o: ../../util/h/headers.h ../../util/h/String.h
visi.xdr.CLNT.o: ../../util/h/Vector.h ../../util/h/Queue.h
visi.xdr.SRVR.o: visi.xdr.SRVR.h visi.xdr.h ../../util/h/rpcUtil.h
visi.xdr.SRVR.o: ../../util/h/headers.h ../../util/h/String.h
visi.xdr.SRVR.o: ../../util/h/Vector.h ../../util/h/Queue.h
client1.o: /usr/include/stdio.h /usr/include/stdlib.h
client1.o: /usr/include/sys/stdtypes.h visi.xdr.CLNT.h visi.xdr.h
client1.o: ../../util/h/rpcUtil.h ../../util/h/headers.h
client1.o: ../../util/h/String.h ../../util/h/Vector.h ../../util/h/Queue.h
server1.o: ../h/visualization.h ../h/datagrid.h /usr/include/string.h
server1.o: /usr/include/sys/stdtypes.h /usr/include/math.h
server1.o: /usr/include/floatingpoint.h /usr/include/sys/ieeefp.h
server1.o: /usr/include/values.h ../h/visiTypes.h /usr/include/stdio.h
server1.o: ../../util/h/String.h /usr/include/assert.h ../../util/h/Vector.h
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
xtext.o: /usr/include/sys/ieeefp.h /usr/include/values.h ../h/visiTypes.h
xtext.o: ../../util/h/String.h /usr/include/assert.h ../../util/h/Vector.h
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
xtext2.o: /usr/include/sys/ieeefp.h /usr/include/values.h ../h/visiTypes.h
xtext2.o: ../../util/h/String.h /usr/include/assert.h ../../util/h/Vector.h
client2.o: /usr/include/stdio.h /usr/include/stdlib.h
client2.o: /usr/include/sys/stdtypes.h visi.xdr.CLNT.h visi.xdr.h
client2.o: ../../util/h/rpcUtil.h ../../util/h/headers.h
client2.o: ../../util/h/String.h ../../util/h/Vector.h ../../util/h/Queue.h
visithread.o: /usr/include/stdio.h visi.xdr.CLNT.h visi.xdr.h
visithread.o: ../../util/h/rpcUtil.h ../../util/h/headers.h
visithread.o: ../../util/h/String.h ../../util/h/Vector.h
visithread.o: ../../util/h/Queue.h
