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
 * smooth.h - header file of smooth.c
 *
 * $Log: smooth.h,v $
 * Revision 1.2  1997/05/21 03:20:35  tung
 * Revised.
 *
 * Revision 1.1  1997/05/12 20:15:42  naim
 * Adding "Terrain" visualization to paradyn (commited by naim, done by tung).
 *
 * Revision 1.1  1992/05/19  07:21:56  lam
 * Initial revision
 *
 *
 */

#ifndef SMOOTH_H
#define SMOOTH_H

void smooth (struct surface_points *new_surface, struct surface_points *surface,
             int wsize);
static float avg_elem (float window[]);
void smooth_med (struct surface_points *new_surface, struct surface_points *surface,
                 int wsize);
static float med_elem (float window[]);
static void shift_win (float window[], float);



#endif
