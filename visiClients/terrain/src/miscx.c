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
 All rights reserved.\n";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/visiClients/terrain/src/miscx.c,v 1.1 1997/05/12 20:15:33 naim Exp $";
#endif

/*
 * miscx.c - Assorted X routines.
 *
 * $Log: miscx.c,v $
 * Revision 1.1  1997/05/12 20:15:33  naim
 * Adding "Terrain" visualization to paradyn (commited by naim, done by tung).
 *
 * Revision 1.1  1992/05/19  17:35:58  lam
 * Initial revision
 *
 * Revision 2.17  1991/09/19  22:28:42  rbi
 * Early multi-application support.
 *
 * Revision 2.16  1991/09/13  15:17:26  hollings
 * moved malloc/realloc declarations to sysver.h
 *
 * Revision 2.15  1991/06/28  16:28:09  rbi
 * onthefly derived metrics.
 *
 * Revision 2.14  1991/03/11  16:30:17  hollings
 * Added check pixmap.
 *
 * Revision 2.13  1991/03/04  22:58:46  hollings
 * PTX Port.
 *
 * Revision 2.12  1990/11/26  17:15:43  hollings
 * Use Athena Menu widgets, and assorted bug fixes.
 *
 * Revision 2.11  90/07/23  15:29:52  hollings
 * Deleted dead code.
 * 
 * Revision 2.10  90/07/13  17:43:09  hollings
 * cleaned up cursors.
 * 
 * Revision 2.9  90/07/12  16:53:19  hollings
 * Added dynamic icons.
 * 
 * Revision 2.8  90/05/14  18:10:02  hollings
 * Toolkit version of req.c
 * 
 * Revision 2.7  90/04/30  18:13:05  hollings
 * Fixed bug with XtError handling.
 * 
 * Revision 2.6  90/02/06  18:15:58  hollings
 * New Copyright
 * 
 * Revision 2.5  90/02/06  18:03:43  hollings
 * New Copyright
 * 
 * Revision 2.4  90/01/31  16:36:58  hollings
 * Added watch cursor for Greyed backgrounds.
 * 
 * Revision 2.3  90/01/23  14:05:09  hollings
 * Added Color Support, and defaults icons
 * 
 * Revision 2.2  89/12/19  16:34:29  hollings
 * X Bug Fixes
 * 
 * Revision 2.1  89/11/06  16:42:37  hollings
 * New Copyright Message
 * 
 * Revision 2.0  89/10/18  17:46:41  hollings
 * Toolkit Release
 * 
 * Revision 1.18  89/10/18  15:32:26  hollings
 * X Toolkit upgrade - Pass #2
 * 
 * Revision 1.17  89/10/02  10:50:13  hollings
 * New Tree Scroll Bar
 * 
 * Revision 1.16  89/09/25  13:27:58  hollings
 * bug fix.
 * 
 * Revision 1.15  89/09/20  18:34:55  hollings
 * X Toolkit integration
 * 
 * Revision 1.14  89/09/07  13:55:28  hollings
 * SPARC port cleanup
 * 
 * Revision 1.13  89/08/23  14:06:30  hollings
 * Big Bug Fix
 * 
 * Revision 1.12  89/08/09  13:33:57  hollings
 * ASYC application abort
 * 
 * Revision 1.11  89/07/26  13:30:11  hollings
 * Assorted bug fixes
 * 
 * Revision 1.10  89/07/20  13:22:33  hollings
 * Added Async X events
 * 
 * Revision 1.9  89/07/14  16:31:30  hollings
 * Fixed phase table for variable fonts
 * 
 *
 */
/* This must be first */
#include "terrain.h"
#include "Xbase.h"
#include "action.h"

#define FONTASCENT ((fontStruct->max_bounds).ascent)

XFontStruct 	*fontStruct;
int 		fontHeight, fontWidth;


PopUpInit()
{
   fontStruct = rv.font;
   fontHeight = vchar;
   fontWidth = XTextWidth(fontStruct, "M", 1);
   InitAction(100,100);
}


IFeep()
{
    XBell(dpy, 0);
}


int
IGetGeometry(win, Info)
Window win;
WindowInfo *Info;
{
    return(XGetGeometry(dpy, win, &(Info->root), &(Info->x), &(Info->y), &(Info->width), &(Info->height), &(Info->bdrwidth), &(Info->depth)));
}


/*
 * malloc the request bytes, and if we can't get them call the IPS error
 *    handler.
 *
 */
char *ips_malloc(count)
int count;
{
   return terrain_alloc(count);
}

/* The X tool kit should have had this function defined  */
void XtGetValue(widget, field, result)                       
Widget widget;
char *field;
caddr_t result;
{
    Arg AList[1];
    
    XtSetArg(AList[0], field, result);
    XtGetValues(widget, AList, 1);
}

/* The X tool kit should have had this function defined  */
void XtSetValue(widget, field, result)                       
Widget widget;
char *field;
caddr_t result;
{
    Arg AList[1];
    
    XtSetArg(AList[0], field, result);
    XtSetValues(widget, AList, 1);
}

















