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
 * $Id: action.h,v 1.6 2001/06/12 19:56:11 schendel Exp $
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
			  struct Logo *Rlogo, void (*callBack)(int));
void InitAction(int x, int y);


#endif
