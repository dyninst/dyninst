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
 * $Id: graph3d.h,v 1.6 1998/03/30 01:22:28 wylie Exp $
 */


#ifndef GRAPH3D_H
#define GRAPH3D_H

extern char *strcpy(),*strncpy(),*strcat();

extern void changeXFormat(int);
extern int do_3dplot();
extern int plot3d_lines();
extern int map3d_xy();





int update_extrema_pts();
int draw_parametric_grid();
int draw_non_param_grid();
int draw_bottom_grid();
int draw_3dxtics();
int draw_3dytics();
int draw_3dztics();
int draw_series_3dxtics();
int draw_series_3dytics();
int draw_series_3dztics();
int draw_set_3dxtics();
int draw_set_3dytics();
int draw_set_3dztics();
int xtick();
int ytick();
int ztick();




#endif
