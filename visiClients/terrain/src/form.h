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
 * $Id: form.h,v 1.4 2001/06/12 19:56:12 schendel Exp $
 */

#ifndef FORM_HEADER
#define FORM_HEADER

Widget createForm(Widget, int, int);
int quit3d(void);
void gotJumpV(Widget, XtPointer, XtPointer);
void gotScrollV(Widget, XtPointer, XtPointer);
void gotJumpH(Widget, XtPointer, XtPointer);
void gotScrollH(Widget, XtPointer, XtPointer);
 
void plot_smooth(void);
void plot_unsmooth(void);   

void plot_usemed(void);
void plot_nomed(void);

void reset_rotate(void);


#endif
