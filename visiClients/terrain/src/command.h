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
 * command.h - header file of command.c.
 *
 * $Log: command.h,v $
 * Revision 1.2  1997/05/20 22:30:52  tung
 * Change the label position when rotating.
 *
 * Revision 1.1  1997/05/12 20:15:24  naim
 * Adding "Terrain" visualization to paradyn (commited by naim, done by tung).
 *
 * Revision 1.1  1992/05/19  07:21:56  lam
 * Initial revision
 *
 *
 */


#define SA_INIT     0
#define SA_JUMP     1
#define SA_ROTATE   2
#define SA_SMOOTH   3
#define SA_ROUGH    4
#define SA_HIDE     5
#define SA_UNHIDE   6
#define SA_RESIZE   7
#define SA_USEMED   8
#define SA_NOMED    9

extern void plot3drequest(/* int action */);
extern void kill_surface();
extern int Graph3DAddNewCurve();
extern void Graph3DSetCurveData();
