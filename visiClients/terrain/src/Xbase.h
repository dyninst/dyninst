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
 * $Log: Xbase.h,v $
 * Revision 1.1  1997/05/12 20:15:18  naim
 * Adding "Terrain" visualization to paradyn (commited by naim, done by tung).
 *
 * Revision 1.1  1992/05/19  17:38:07  lam
 * Initial revision
 *
 *
 * Revision 2.4  1991/04/03  20:56:09  hollings
 * Removed duplicate include files.
 *
 * Revision 2.3  90/02/06  18:16:49  hollings
 * New Copyright
 * 
 * Revision 2.2  90/02/06  18:04:33  hollings
 * New Copyright
 * 
 * Revision 2.1  89/11/06  16:54:26  hollings
 * New copyright message
 * 
 *
 */

#define ROOTWINDOW 1
#define BASEGCV (GCFont | GCForeground | GCBackground | GCLineWidth)

/*
 * 	Window info returned by XGetGeometry
 */
typedef struct _WindowInfo {
	int width, height;	/* Width and height. */
	int x, y;		/* X and y coordinates. */
	int bdrwidth;		/* Border width. */
	int depth;		/* depth of window */
	short mapped;		/* NOT USED: IsUnmapped, IsMapped or IsInvisible.*/
	short type;		/* NOT USED: IsTransparent, IsOpaque or IsIcon. */
	Window assoc_wind;	/* NOT USED: Associated icon or opaque Window. */
	Window root;		/* root of screen containing window */
} WindowInfo;

extern void XtGetValue();
