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
 * $Id: graph3d.h,v 1.8 2001/06/12 19:56:12 schendel Exp $
 */


#ifndef GRAPH3D_H
#define GRAPH3D_H

/* extern char *strcpy(),*strncpy(),*strcat();*/
#include <string.h>
#include "plot.h"

extern void changeXFormat(int);
extern int do_3dplot(struct surface_points *plots, int pcount, double min_x, 
		     double max_x, double min_y, double max_y, double min_z, 
		     double max_z);
extern int plot3d_lines(struct surface_points *plot, int printIndex);
extern int map3d_xy(double x, double y, double z, int *xt, int *yt);





int update_extrema_pts(int ix, int iy, int *min_sx_x, int *min_sx_y, 
			      int *min_sy_x, int *min_sy_y,
			      double x, double y);
int draw_parametric_grid(double z_min);
int draw_bottom_grid(struct surface_points *plot, double min_z, 
			    double max_z);
int draw_3dxtics(double start, double incr, double end, double ypos, 
		 double z_min);
int draw_3dytics(double start, double incr, double end, double xpos, 
		 double z_min);
int draw_3dztics(double start, double incr, double end, double xpos, 
		 double ypos, double z_min, double z_max);
int draw_series_3dxtics(double start, double incr, double end, double ypos, 
			double z_min);
int draw_series_3dytics(double start, double incr, double end, double xpos, 
			double z_min);
int draw_series_3dztics(double start, double incr, double end, double xpos, 
			double ypos, double z_min, double z_max);
int draw_set_3dxtics(struct ticmark *list, double ypos, double z_min);
int draw_set_3dytics(struct ticmark *list, double xpos, double z_min);
int draw_set_3dztics(struct ticmark *list, double xpos, double ypos, 
		     double z_min, double z_max);
int xtick(double place, char *text, double spacing, double ticscale, 
	  double ypos, double z_min);
int ytick(double place, char *text, double spacing, double ticscale, 
	  double xpos, double z_min);
int ztick(double place, char *text, double spacing, double ticscale, 
	  double xpos, double ypos);




#endif
