
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
 * $Log: menu.h,v $
 * Revision 1.1  1997/05/12 20:15:31  naim
 * Adding "Terrain" visualization to paradyn (commited by naim, done by tung).
 *
 * Revision 1.1  1992/05/19  07:21:56  lam
 * Initial revision
 *
 *
 * Revision 1.3  1991/05/15  19:43:42  hollings
 * Added support for lines in menus.
 *
 * Revision 1.2  1991/03/11  16:30:17  hollings
 * Added Advanced menus via CreateAdvancedMenu interface.
 *
 * Revision 1.1  1990/11/26  17:15:31  hollings
 * Initial revision
 *
 */

/*
 * A single item in a menu.
 *
 */
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
Widget CreateMenu();


/* more complex interface required to get back widgets for each item or
 *  to set margins.
 */
Widget CreateAdvancedMenu();

/*
 * Psuedo menu items.
 *
 */
#define 	MENU_LINE	(char *) 1
