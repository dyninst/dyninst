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
 * $Id: Xglobals.h,v 1.2 1998/03/30 01:22:17 wylie Exp $
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
