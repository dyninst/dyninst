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

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/visiClients/terrain/src/menu.c,v 1.3 1997/05/21 02:27:28 tung Exp $";
#endif

/*
 * menu.c - menu handler code.
 *
 * $Log: menu.c,v $
 * Revision 1.3  1997/05/21 02:27:28  tung
 * Revised.
 *
 * Revision 1.2  1997/05/19 01:00:09  tung
 * Eliminate ips dependent library files.
 *
 * Revision 1.1  1997/05/12 20:15:31  naim
 * Adding "Terrain" visualization to paradyn (commited by naim, done by tung).
 *
 * Revision 1.1  1992/05/19  16:29:59  lam
 * Initial revision
 *
 * Revision 1.2  1992/03/09  04:24:18  lam
 * Use terrain_alloc instead of ips_malloc.
 *
 * Revision 1.1  1992/02/28  04:43:50  lam
 * Initial revision
 *
 * Revision 2.7  1991/05/15  19:41:09  hollings
 * Assorted header fixes.
 *
 * Revision 2.6  1991/03/11  16:30:17  hollings
 * Added Advanced menus via CreateAdvancedMenu interface.
 *
 * Revision 2.5  1990/11/26  17:15:28  hollings
 * Use Athena Menu widgets, and assorted bug fixes.
 *
 * Revision 2.4  90/02/06  18:15:52  hollings
 * New Copyright
 * 
 * Revision 2.3  90/02/06  18:03:37  hollings
 * New Copyright
 * 
 * Revision 2.2  89/11/29  14:21:04  hollings
 * Various small bug fixes
 * 
 * Revision 2.1  89/11/06  16:42:32  hollings
 * New Copyright Message
 * 
 * Revision 2.0  89/10/18  17:46:36  hollings
 * Toolkit Release
 * 
 * Revision 1.4  89/09/20  18:34:51  hollings
 * X Toolkit integration
 * 
 * Revision 1.3  89/09/07  13:55:26  hollings
 * SPARC port cleanup
 * 
 * Revision 1.2  89/08/04  16:04:21  hollings
 * Aded calls to ips_malloc
 * 
 *
 */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/Sme.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/SmeBSB.h>

#include "menu.h"
#include "misc.h"

Widget CreateAdvancedMenu(def)
struct menuDefintion *def;
{
    int i;
    int total;
    int acount;
    Arg args[100];
    char name[100];
    Widget menu, entry;

    acount = 0;
    //XtSetArg(args[acount], XtNlabel, def->title); acount++;
    menu = XtCreatePopupShell("menu", simpleMenuWidgetClass, def->parent, 
	args, acount);

    for (i=0, total = 0; def->data[i].name; i++, total++);
    def->items = (Widget *) terrain_alloc(sizeof(Widget) * total);

    for (i=0; def->data[i].name; i++) {
	acount = 0;
	if (def->data[i].name == MENU_LINE) {
	    entry = XtCreateManagedWidget("line", smeLineObjectClass,
		menu, args, acount);
	    def->items[i] = entry;
	} else {
	    XtSetArg(args[acount], XtNlabel, def->data[i].name); acount++;
	    XtSetArg(args[acount], XtNleftMargin, def->leftMargin); acount++;
	    XtSetArg(args[acount], XtNrightMargin, def->rightMargin); acount++;
	    entry = XtCreateManagedWidget(def->data[i].name, smeBSBObjectClass, 
		menu, args, acount);
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
