/*
 * Copyright (c) 1989, 1990 Barton P. Miller, Morgan Clark, Timothy Torzewski
 *     Jeff Hollingsworth, and Bruce Irvin. All rights reserved.
 *
 * This software is furnished under the condition that it may not
 * be provided or otherwise made available to, or used by, any
 * other person.  No title to or ownership of the software is
 * hereby transferred.  The name of the principals
 * may not be used in any advertising or publicity related to this
 * software without specific, written prior authorization.
 * Any use of this software must include the above copyright notice.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1989, 1990 Barton P. Miller,\
 Morgan Clark, Timothy Torzewski, Jeff Hollingsworth, and Bruce Irvin.\
 All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/visiClients/terrain/src/action.c,v 1.1 1997/05/12 20:15:20 naim Exp $";
#endif


/*
 * This file contains the routines to display an action box.
 *   An action box consists of several lines of text followed by a row
 *   buttons defining the possible actions to take in responce to the
 *   message.
 *
 * $Log: action.c,v $
 * Revision 1.1  1997/05/12 20:15:20  naim
 * Adding "Terrain" visualization to paradyn (commited by naim, done by tung).
 *
 * Revision 1.1  1992/05/19  17:29:29  lam
 * Initial revision
 *
 *
 * Revision 2.14  1991/03/14  20:48:17  hollings
 * Fixed $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/visiClients/terrain/src/action.c,v 1.1 1997/05/12 20:15:20 naim Exp $ definition.
 *
 * Revision 2.13  1990/08/24  13:01:40  hollings
 * Added include of <X11/Intrinsic.h>.
 *
 * Revision 2.12  90/08/10  17:07:06  hollings
 * Changed name of banner label widget.
 * 
 * Revision 2.11  90/05/23  11:39:00  hollings
 * Moved some X parameters to the application defaults file.
 * 
 * Revision 2.10  90/05/18  15:39:50  hollings
 * Fixed window placement for Motif Window manager.
 * 
 * Revision 2.9  90/05/17  15:45:04  hollings
 * Small bug fixes with icons.
 * 
 * Revision 2.8  90/05/16  15:49:49  hollings
 * Conversion to use toolkit.
 * 
 * Revision 2.7  90/04/20  13:14:15  hollings
 * Added include of ips.h.
 * 
 * Revision 2.6  90/02/15  19:57:03  hollings
 * Fixed edit bug.
 * 
 * Revision 2.5  90/02/06  18:14:56  hollings
 * New Copyright
 * 
 * Revision 2.4  90/02/06  18:02:47  hollings
 * New Copyright
 * 
 * Revision 2.3  89/11/06  16:41:33  hollings
 * New Copyright Message
 * 
 * Revision 2.2  89/10/27  17:10:01  hollings
 * Various Bug Fixes
 * 
 * Revision 2.1  89/10/19  14:10:18  hollings
 * Fixed banner offset.
 * 
 * Revision 2.0  89/10/18  17:45:46  hollings
 * Toolkit Release
 * 
 * Revision 1.4  89/10/18  15:31:43  hollings
 * X Toolkit upgrade - Pass #2
 * 
 * Revision 1.3  89/08/23  14:08:12  hollings
 * Big Bug Fix
 * 
 * Revision 1.2  89/08/09  13:33:37  hollings
 * ASYC application abort
 * 
 * Revision 1.1  89/07/26  13:26:39  hollings
 * Initial revision
 * 
 */
#include <config.h>

#define max(x,y) 	((x > y) ? x : y);
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/AsciiText.h>


#include <FormatBox.h>
#include <Xbase.h>
#include <Xglobals.h>
#include <action.h>
#include "terrain.h"

static int Xorig, Yorig;

struct ActData {
    int active;			/* still waiting on a responce */
    Widget shell;		/* top level holder */
    Widget form;		/* form for items */
    Widget banner;		/* banner string */
    Widget Llogo;		/* left logo */
    Widget Rlogo;		/* right logo */
    Widget box;			/* box to hold buttons */
    Widget *buttons;		/* buttons */
    Widget message;		/* message */
    char *str;			/* text of message */
    void (*callBack)();		/* func to call when we are done */
};


#define EXPANSION       (5 * fontWidth)
#define DEF_DIST        15

Pixmap icon_pixmap = NULL;

static int width, height;
static struct ActData act;
static int start_x, start_y;
static Arg get_stuff[] = {
    {    XtNwidth,      (XtArgVal) &width },
};

static EndActFunc(w, id, cdata)
Widget w;
int id;
int cdata;
{
    XtRemoveGrab(act.shell);
    XtUnmapWidget(act.shell);
    if (act.callBack) (act.callBack)(id);
    XtDestroyWidget(act.shell);
    act.active = 0;
    if (act.message) free(act.message);
    return;
}

RequestAction(banner, Llogo, Rlogo, labelc, labels, buttonc, buttons, justify, callBack)
int labelc;
int buttonc;
int justify;
char *banner;
char *labels[];
char *buttons[];
struct Logo *Llogo, *Rlogo;
void (*callBack)();
{

    int i;
    int b_y;
    int count;
    int leng;
    int position;
    void IFeep();
    XEvent event;
    Arg args[200];
    WindowInfo info;
    XSizeHints hints;
    Widget last = NULL;

    act.active = 1;
    act.callBack = callBack;

    count = 0;
    XtSetArg(args[count], XtNx, start_x); count++;
    XtSetArg(args[count], XtNy, start_y); count++;
    XtSetArg(args[count], XtNiconName, "Terrain Message"); count++;
    XtSetArg(args[count], XtNmappedWhenManaged, False); count++;
    act.shell = XtCreatePopupShell("Message" , topLevelShellWidgetClass,
        w_top, args, count);

    /* form to hold items */
    count = 0;
    XtSetArg(args[count], XtNdefaultDistance, DEF_DIST); count++;
    act.form = XtCreateManagedWidget("form", formWidgetClass,
        act.shell, args, count);
    XtAddEventHandler(act.form, ButtonPressMask, NULL, IFeep, NULL);

    /* left logo if it exists */
    if (Llogo) {
        count = 0;
	XtSetArg(args[count], XtNborderWidth, 0); count++;
        XtSetArg(args[count], XtNbitmap, Llogo->icon); count++;
	last = act.Llogo = XtCreateManagedWidget("Llogo", labelWidgetClass,
	    act.form, args, count);
    }

    /* banner title */
    b_y = Llogo ? Llogo->h : 0;
    b_y = (Rlogo && (Rlogo->h > b_y)) ? Rlogo->h : b_y;
    count = 0;
    if (Llogo) XtSetArg(args[count], XtNhorizDistance, 0), count++;
    if (b_y) XtSetArg(args[count], XtNheight, b_y); count++;
    XtSetArg(args[count], XtNresizable, True); count++;
    XtSetArg(args[count], XtNresize, True); count++;
    XtSetArg(args[count], XtNfromHoriz, last); count++;
    XtSetArg(args[count], XtNborderWidth, 0); count++;
    XtSetArg(args[count], XtNlabel, banner); count++;
    last = act.banner = XtCreateManagedWidget("bannerString", labelWidgetClass,
	act.form, args, count);

    /* right logo if it exists */
    if (Rlogo) {
        count = 0;
        XtSetArg(args[count], XtNbitmap, Rlogo->icon); count++;
	XtSetArg(args[count], XtNborderWidth, 0); count++;
	XtSetArg(args[count], XtNfromHoriz, act.banner); count++;
	XtSetArg(args[count], XtNhorizDistance, 5), count++;
	last = act.Rlogo = XtCreateManagedWidget("Rlogo", labelWidgetClass,
	    act.form, args, count);
    }

    if (labelc) {
	leng = 0;
	width = 0;
	for (i=0; i < labelc; i++) {
	    leng += strlen(labels[i]);
	    width = max(width,XTextWidth(fontStruct, labels[i], 
					 strlen(labels[i])));
	}
	act.str = (char *) ips_malloc(leng + labelc);
	act.str[0] = '\0';
	for (i=0; i < labelc; i++) {
	    if (i) strcat(act.str, "\n");
	    strcat(act.str, labels[i]);
	}

	count = 0;
	XtSetArg(args[count], XtNborderWidth, 0); count++;
	XtSetArg(args[count], XtNtopMargin, 0); count++;
	XtSetArg(args[count], XtNbottomMargin, 0); count++;
	XtSetArg(args[count], XtNwidth, width+10); count++;
	height = fontHeight * labelc;
	XtSetArg(args[count], XtNheight, height+5); count++;
	XtSetArg(args[count], XtNjustify, justify); count++;
	XtSetArg(args[count], XtNfromVert, last); count++;
	XtSetArg(args[count], XtNresizable, True); count++;
	XtSetArg(args[count], XtNdisplayCaret, False); count++;
	XtSetArg(args[count], XtNresize, XawtextResizeWidth); count++;
	XtSetArg(args[count], XtNstring, act.str); count++;
	last = act.message = XtCreateManagedWidget("message", 
	    asciiTextWidgetClass, act.form, args, count);
    }
	
    count = 0;
    XtSetArg(args[count], XtNfromVert, last); count++;
    XtSetArg(args[count], XtNfromHoriz, NULL); count++;
    XtSetArg(args[count], XtNborderWidth, 0); count++;
    XtSetArg(args[count], XtNvSpace, 5); count++;
    XtSetArg(args[count], XtNhSpace, 15); count++;
    XtSetArg(args[count], XtNorientation, XtorientHorizontal); count++;
    XtSetArg(args[count], XtNtop, XtChainBottom); count++;
    XtSetArg(args[count], XtNbottom, XtChainBottom); count++;
    XtSetArg(args[count], XtNresizable, True); count++;
    XtSetArg(args[count], XtNresize, True); count++;
    XtSetArg(args[count], XtNformat, XtNcenter); count++;
    act.box = XtCreateManagedWidget("box", formatBoxWidgetClass, 
        act.form, args, count);

    act.buttons = (Widget *) calloc(buttonc, sizeof(Widget));
    for (i=0; i < buttonc; i++) {
        last = act.buttons[i] = XtCreateManagedWidget(buttons[i], 
            commandWidgetClass, act.box, NULL, 0);
        XtAddCallback(act.buttons[i], XtNcallback, EndActFunc, i);
    }

    XtRealizeWidget(act.shell);
    IGetGeometry(XtWindow(act.form), &info);
    width = info.width - 2 * DEF_DIST;
    width -= Rlogo ? (Rlogo->w + DEF_DIST): 0;
    width -= Llogo ? (Llogo->w + DEF_DIST): 0;
    XtSetValue(act.banner, XtNwidth, width);

    width = info.width - 2 * DEF_DIST;
    if (labelc) XtSetValue(act.message, XtNwidth, width);
    XtSetValue(act.box, XtNwidth, width);

    hints.flags = USPosition;
    hints.x = start_x;
    hints.y = start_y;
    XSetNormalHints(XtDisplay(act.shell), XtWindow(act.shell), &hints);
    XtAddGrab(act.shell, TRUE, FALSE);
    XtMapWidget(act.shell);

    /* We need to process events until the action is done, then we can let
       the function return.  This prevents multiple error windows from becomming
       active at once. */
    while (act.active) {
	XNextEvent(dpy, &event);
	XtDispatchEvent(&event);
    }
    return(0);
}

InitAction(x, y)
int x, y;
{
    start_x = x;
    start_y = y;
}




