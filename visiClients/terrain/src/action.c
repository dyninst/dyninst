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

/*
 * This file contains the routines to display an action box.
 *   An action box consists of several lines of text followed by a row
 *   buttons defining the possible actions to take in responce to the
 *   message.
 *
 * $Id: action.c,v 1.9 2002/02/11 22:08:00 tlmiller Exp $
 */

#define max(x,y) 	((x > y) ? x : y);

#ifdef i386_unknown_linux2_0 || defined(ia64_unknown_linux2_4)
#define _HAVE_STRING_ARCH_strcpy  /* gets rid of warnings */
#define _HAVE_STRING_ARCH_strsep
#endif
#define XTSTRINGDEFINES

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
#include <stdlib.h>

#include "FormatBox.h"
#include "Xbase.h"
#include "Xglobals.h"
#include "action.h"
#include "terrain.h"
#include "miscx.h"

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
    void (*callBack)(int);	/* func to call when we are done */
};


#define EXPANSION       (5 * fontWidth)
#define DEF_DIST        15

Pixmap icon_pixmap = (Pixmap)NULL;

static int width, height;
static struct ActData act;
static int start_x, start_y;

void EndActFunc(Widget w, int id, int cdata)
{
    // garbage, just to remove warning
    void *ptr = w; if(ptr==NULL) { cdata=cdata; } 
    XtRemoveGrab(act.shell);
    XtUnmapWidget(act.shell);
    if (act.callBack) (act.callBack)(id);
    XtDestroyWidget(act.shell);
    act.active = 0;
    if (act.message) 
       free(act.message);
    return;
}

int RequestAction(int labelc, int buttonc, int justify, char *banner, char *labels[],
                   char *buttons[], struct Logo *Llogo, struct Logo *Rlogo, 
                   void (*callBack)(int))
{

    int i;
    int b_y;
    int count;
    int leng;
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
        w_top, args, (unsigned) count);

    /* form to hold items */
    count = 0;
    XtSetArg(args[count], XtNdefaultDistance, DEF_DIST); count++;
    act.form = XtCreateManagedWidget("form", formWidgetClass,
        act.shell, args, (unsigned) count);
    XtAddEventHandler(act.form, ButtonPressMask, (int)NULL, IFeep, NULL);

    /* left logo if it exists */
    if (Llogo) {
        count = 0;
	XtSetArg(args[count], XtNborderWidth, 0); count++;
        XtSetArg(args[count], XtNbitmap, Llogo->icon); count++;
	last = act.Llogo = XtCreateManagedWidget("Llogo", labelWidgetClass,
	    act.form, args, (unsigned) count);
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
	act.form, args, (unsigned) count);

    /* right logo if it exists */
    if (Rlogo) {
        count = 0;
        XtSetArg(args[count], XtNbitmap, Rlogo->icon); count++;
	XtSetArg(args[count], XtNborderWidth, 0); count++;
	XtSetArg(args[count], XtNfromHoriz, act.banner); count++;
	XtSetArg(args[count], XtNhorizDistance, 5), count++;
	last = act.Rlogo = XtCreateManagedWidget("Rlogo", labelWidgetClass,
	    act.form, args, (unsigned) count);
    }

    if (labelc) {
	leng = 0;
	width = 0;
	for (i=0; i < labelc; i++) {
	    leng += strlen(labels[i]);
	    width = max(width,XTextWidth(fontStruct, labels[i], 
					 (signed) strlen(labels[i])));
	}
	act.str = (char *) malloc((unsigned) leng + labelc);
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
	    asciiTextWidgetClass, act.form, args, (unsigned) count);
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
        act.form, args, (unsigned) count);

    act.buttons = (Widget *) calloc((unsigned) buttonc, sizeof(Widget));
    for (i=0; i < buttonc; i++) {
        last = act.buttons[i] = XtCreateManagedWidget(buttons[i], 
            commandWidgetClass, act.box, NULL, 0);
        XtAddCallback(act.buttons[i], XtNcallback, (XtCallbackProc)EndActFunc,
	             (XtPointer)i);
    }

    XtRealizeWidget(act.shell);
    IGetGeometry(XtWindow(act.form), &info);
    width = info.width - 2 * DEF_DIST;
    width -= Rlogo ? (Rlogo->w + DEF_DIST): 0;
    width -= Llogo ? (Llogo->w + DEF_DIST): 0;
    XtSetValue(act.banner, XtNwidth, (XtPointer)width);

    width = info.width - 2 * DEF_DIST;
    if (labelc) XtSetValue(act.message, XtNwidth, (XtPointer)width);
    XtSetValue(act.box, XtNwidth, (XtPointer)width);

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

void InitAction(int x, int y)
{
    start_x = x;
    start_y = y;
}




