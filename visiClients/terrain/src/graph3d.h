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
 * graph3d.h - header file for graph3d.c
 *
 * $Log: graph3d.h,v $
 * Revision 1.4  1997/05/21 02:27:28  tung
 * Revised.
 *
 * Revision 1.2  1997/05/20 08:29:19  tung
 * Revised on resizing the maxZ, change the xlabel and zlabel format.
 *
 * Revision 1.1  1997/05/12 20:15:29  naim
 * Adding "Terrain" visualization to paradyn (commited by naim, done by tung).
 *
 * Revision 1.1  1992/05/19  07:21:56  lam
 * Initial revision
 *
 *
 */


#ifndef GRAPH3D_H
#define GRAPH3D_H

extern char *strcpy(),*strncpy(),*strcat();

extern void changeXFormat(int);
extern void do_3dplot();
extern void plot3d_lines();
extern int map3d_xy();


#endif
