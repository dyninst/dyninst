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
 * form.h - header file of form.c
 *
 * $Log: form.h,v $
 * Revision 1.2  1997/05/21 02:27:26  tung
 * Revised.
 *
 * Revision 1.1  1997/05/12 20:15:27  naim
 * Adding "Terrain" visualization to paradyn (commited by naim, done by tung).
 *
 * Revision 1.1  1992/05/19  07:21:56  lam
 * Initial revision
 *
 *
 */

#ifndef FORM_HEADER
#define FORM_HEADER

Widget createForm(Widget, int, int);
int quit3d();
void gotJumpV(Widget, XtPointer, XtPointer);
void gotScrollV(Widget, XtPointer, XtPointer);
void gotJumpH(Widget, XtPointer, XtPointer);
void gotScrollH(Widget, XtPointer, XtPointer);
 
void plot_smooth();
void plot_unsmooth();   

void plot_usemed();
void plot_nomed();

void reset_rotate();


#endif
