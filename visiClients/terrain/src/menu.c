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
 * menu.c - menu handler code.
 *
 * $Id: menu.c,v 1.7 2002/02/11 22:08:06 tlmiller Exp $
 */

#ifdef i386_unknown_linux2_0 || defined(ia64_unknown_linux2_4)
#define _HAVE_STRING_ARCH_strcpy  /* gets rid of warnings */
#define _HAVE_STRING_ARCH_strsep
#endif

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/Sme.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/SmeBSB.h>
#include <stdlib.h>

#include "menu.h"
#include "misc.h"

Widget CreateAdvancedMenu(def)
struct menuDefintion *def;
{
    int i;
    int total;
    int acount;
    Arg args[100];
    Widget menu, entry;

    acount = 0;
    menu = XtCreatePopupShell("menu", simpleMenuWidgetClass, def->parent, 
	args, (unsigned)acount);

    for (i=0, total = 0; def->data[i].name; i++, total++);
    def->items = (Widget *) terrain_alloc(sizeof(Widget) * total);

    for (i=0; def->data[i].name; i++) {
	acount = 0;
	if (def->data[i].name == MENU_LINE) {
	    entry = XtCreateManagedWidget("line", smeLineObjectClass,
		menu, args, (unsigned)acount);
	    def->items[i] = entry;
	} else {
	    XtSetArg(args[acount], XtNlabel, def->data[i].name); acount++;
	    XtSetArg(args[acount], XtNleftMargin, def->leftMargin); acount++;
	    XtSetArg(args[acount], XtNrightMargin, def->rightMargin); acount++;
	    entry = XtCreateManagedWidget(def->data[i].name, smeBSBObjectClass, 
		menu, args, (unsigned)acount);
	    def->items[i] = entry;
	    /* fix call back data */
	    XtAddCallback(entry, XtNcallback, def->data[i].func, def->cdata);
	}
    }
    XtRealizeWidget(menu);
    return(menu);
}

/*
 * Simple interface to creating menus.  Used for menu that don't have extra
 *   bitmaps to the left/right or need to get back a list of widgets for each
 *   item in the menu.
 *
 */
Widget CreateMenu(parent, title, data, cdata)
Widget parent;
char *title;
struct menuItem *data;
caddr_t cdata;
{
    Widget menu;
    struct menuDefintion stuff;

    stuff.title = title;
    stuff.parent = parent;
    stuff.data = data;
    stuff.cdata = cdata;
    stuff.leftMargin = 4;
    stuff.rightMargin = 4;
    menu = CreateAdvancedMenu(&stuff);
    free(stuff.items);
    return(menu);
}

void DestroyMenu(Widget currMenu)
{
    XtDestroyWidget(currMenu);
}
