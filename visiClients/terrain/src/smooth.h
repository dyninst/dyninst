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
 * $Id: smooth.h,v 1.6 2006/05/02 14:49:30 darnold Exp $
 */

#ifndef SMOOTH_H
#define SMOOTH_H

void smooth (struct surface_points *new_surface, struct surface_points *surface,
             int wsize);
void smooth_med (struct surface_points *new_surface, struct surface_points *surface,
                 int wsize);


#endif
