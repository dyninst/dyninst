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
 * $Id: Xbase.h,v 1.3 2001/06/12 19:56:11 schendel Exp $
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

extern void XtGetValue(Widget widget, char *field, caddr_t result);

