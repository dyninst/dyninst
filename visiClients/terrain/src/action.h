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
 * action.h - Header file for RequestAction Display.
 *
 * $Log: action.h,v $
 * Revision 1.4  1997/05/21 02:27:21  tung
 * Revised.
 *
 * Revision 1.3  1997/05/19 16:03:40  tung
 * Remove unused files.
 *
 * Revision 1.2  1997/05/19 01:00:07  tung
 * Eliminate ips dependent library files.
 *
 * Revision 1.1  1997/05/12 20:15:21  naim
 * Adding "Terrain" visualization to paradyn (commited by naim, done by tung).
 *
 * Revision 1.1  1992/05/19  17:38:44  lam
 * Initial revision
 *
 *
 * Revision 2.4  90/05/16  15:49:57  hollings
 * Conversion to use toolkit.
 * 
 * Revision 2.3  90/02/06  18:17:08  hollings
 * New Copyright
 * 
 * Revision 2.2  90/02/06  18:04:38  hollings
 * New Copyright
 * 
 * Revision 2.1  89/11/06  16:54:30  hollings
 * New copyright message
 * 
 * Revision 2.0  89/10/18  17:47:42  hollings
 * Toolkit Release
 * 
 * Revision 1.2  89/10/18  15:31:48  hollings
 * X Toolkit upgrade - Pass #2
 * 
 * Revision 1.1  89/08/23  14:08:15  hollings
 * Big Bug Fix
 * 
 */


#ifndef ACTION_H
#define ACTION_H

struct Logo {
    Pixmap icon;
    int w, h;
};

void EndActFunc(Widget w, int id, int cdata);
int  RequestAction(int labelc, int buttonc, int justify, char *banner, 
		   	  char *labels[], char *buttons[], struct Logo *Llogo, 
			  struct Logo *Rlogo, void (*callBack)());
void InitAction(int x, int y);


#endif
