/* 
 * Copyright (c) 1989, 1990 Barton P. Miller, Morgan Clark, Timothy Torzewski,
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
 * $Id: menu.h,v 1.4 1998/03/30 01:22:29 wylie Exp $
 */

/*
 * A single item in a menu.
 */

#ifndef MENU_H
#define MENU_H


struct menuItem {
    char *name;
    void (*func)();
};

/*
 * passed to CreateAdvancedMenu.  This defines attributes of advanced menus.
 *
 */
struct menuDefintion {
    char *title;
    Widget parent;
    struct menuItem *data;
    Widget *items;
    caddr_t cdata;
    int leftMargin, rightMargin;
};

/* Simple interface to menu code. */
Widget CreateMenu(Widget, char*, struct menuItem*, caddr_t);
void DestroyMenu(Widget currMenu);

/* more complex interface required to get back widgets for each item or
 *  to set margins.
 */
Widget CreateAdvancedMenu(struct menuDefintion *);

/*
 * Psuedo menu items.
 *
 */
#define 	MENU_LINE	(char *) 1



#endif
