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
 * $Log: Xglobals.h,v $
 * Revision 1.1  1997/05/12 20:15:19  naim
 * Adding "Terrain" visualization to paradyn (commited by naim, done by tung).
 *
 * Revision 1.1  1992/05/19  17:38:26  lam
 * Initial revision
 *
 *
 * Revision 2.3  90/02/06  18:17:04  hollings
 * New Copyright
 * 
 * Revision 2.2  90/02/06  18:04:35  hollings
 * New Copyright
 * 
 * Revision 2.1  89/11/06  16:54:29  hollings
 * New copyright message
 * 
 *
 */

#define FONTASCENT ((fontStruct->max_bounds).ascent)

extern Display *display;
extern Window   IRootWindow;
extern int 	scrn;
extern int 	DWidth;
extern int 	DHeight;
extern GC	baseGC;
extern GC	reverseGC;
extern Font		font;
extern XFontStruct 	*fontStruct;
extern int		fontHeight, fontWidth;
