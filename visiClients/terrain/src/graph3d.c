/* GNUPLOT - graph3d.c */
/*
 * Copyright (C) 1986, 1987, 1990   Thomas Williams, Colin Kelley
 *
 * Permission to use, copy, and distribute this software and its
 * documentation for any purpose with or without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and 
 * that both that copyright notice and this permission notice appear 
 * in supporting documentation.
 *
 * Permission to modify the software is granted, but not the right to
 * distribute the modified code.  Modifications are to be distributed 
 * as patches to released version.
 *  
 * This software  is provided "as is" without express or implied warranty.
 * 
 *
 * AUTHORS
 * 
 *   Original Software:
 *     Thomas Williams,  Colin Kelley.
 * 
 *   Gnuplot 2.0 additions:
 *       Russell Lang, Dave Kotz, John Campbell.
 *
 *   3D extensions:
 *	Gershon Elber.
 * 
 * send your comments or suggestions to (pixar!info-gnuplot@sun.com).
 * 
 */

/*
 * Adapted for terrain.c:
 *      Chi-Ting Lam.
 *
 * $Id: graph3d.c,v 1.10 2001/06/12 19:56:12 schendel Exp $
 */

#ifdef i386_unknown_linux2_0
#define _HAVE_STRING_ARCH_strcpy  /* gets rid of warnings */
#define _HAVE_STRING_ARCH_strsep
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "terrain.h"
#include "plot.h"
#include "colorize.h"
#include "setshow.h"
#include "graph3d.h"


static int timeAxis = 0;  /* 0 - display as min:sec  */
			  /* 1 - display as hour:min */

#ifndef max		/* Lattice C has max() in math.h, but shouldn't! */
#define max(a,b) ((a > b) ? a : b)
#endif

#ifndef min
#define min(a,b) ((a < b) ? a : b)
#endif

#define inrange(z,min,max) ((min<max) ? ((z>=min)&&(z<=max)) : ((z>=max)&&(z<=min)) )

#define apx_eq(x,y) (fabs(x-y) < 0.001)
#define abs(x) ((x) >= 0 ? (x) : -(x))
#define sqr(x) ((x) * (x))

/* Define the boundary of the plot
 * These are computed at each call to do_plot, and are constant over
 * the period of one do_plot. They actually only change when the term
 * type changes and when the 'set size' factors change. 
 */
/* static */ int xleft, xright, ybot, ytop, xmiddle, ymiddle, xscaler, yscaler;

/* Boundary and scale factors, in user coordinates */
/* x_min3d, x_max3d, y_min3d, y_max3d, z_min3d, z_max3d are local to this
 * file and are not the same as variables of the same names in other files
 */
/* static */float x_min3d, x_max3d, y_min3d, y_max3d, z_min3d, z_max3d;
/* static */float xscale3d, yscale3d, zscale3d;
/* static */float real_z_min3d, real_z_max3d;
/* static */float min_sy_ox,min_sy_oy; /* obj. coords. for xy tics placement. */
/* static */float min_sx_ox,min_sx_oy; /* obj. coords. for z tics placement. */

typedef float transform_matrix[16];
/* static */transform_matrix trans_mat;
#define matCell(i,j) (((i) << 2) + (j))


/* (DFK) Watch for cancellation error near zero on axes labels */
#define SIGNIF (0.01)		/* less than one hundredth of a tic mark */
#define CheckZero(x,tic) (fabs(x) < ((tic) * SIGNIF) ? 0.0 : (x))
#define NearlyEqual(x,y,tic) (fabs((x)-(y)) < ((tic) * SIGNIF))

/* And the functions to map from user to terminal coordinates */
#define map_x(x) (int)(x+0.5) /* maps floating point x to screen */ 
#define map_y(y) (int)(y+0.5)	/* same for y */

/* And the functions to map from user 3D space into normalized -1..1 */
#define map_x3d(x) ((x-x_min3d)*xscale3d-1.0)
#define map_y3d(y) ((y-y_min3d)*yscale3d-1.0)
#define map_z3d(z) ((z-z_min3d)*zscale3d-1.0)

void changeXFormat(format)
int format;
{
   timeAxis = format;
}

static int mat_unit(transform_matrix mat)
{
    int i, j;

    for (i = 0; i < 4; i++) for (j = 0; j < 4; j++)
	if (i == j)
	    mat[matCell(i,j)] = 1.0;
	else
	    mat[matCell(i,j)] = 0.0;

    return 0;

}

static int mat_trans(double tx, double ty, double tz, transform_matrix mat)
{
     mat_unit(mat);                                 /* Make it unit matrix. */
     mat[matCell(3,0)] = tx;
     mat[matCell(3,1)] = ty;
     mat[matCell(3,2)] = tz;

  return 0;
}

static int mat_scale(double sx, double sy, double sz, transform_matrix mat)
{
     mat_unit(mat);                                 /* Make it unit matrix. */
     mat[matCell(0,0)] = sx;
     mat[matCell(1,1)] = sy;
     mat[matCell(2,2)] = sz;

    return 0;
}

static int mat_rot_x(double teta, transform_matrix mat)
{
    float cos_teta, sin_teta;

    teta *= Pi / 180.0;
    cos_teta = cos((double) teta);
    sin_teta = sin((double) teta);

    mat_unit(mat);                                  /* Make it unit matrix. */
    mat[matCell(1,1)] = cos_teta;
    mat[matCell(1,2)] = -sin_teta;
    mat[matCell(2,1)] = sin_teta;
    mat[matCell(2,2)] = cos_teta;
   
return 0;
}

static int mat_rot_y(double teta, transform_matrix mat)
{
    float cos_teta, sin_teta;

    teta *= Pi / 180.0;
    cos_teta = cos((double) teta);
    sin_teta = sin((double) teta);

    mat_unit(mat);                                  /* Make it unit matrix. */
    mat[matCell(0,0)] = cos_teta;
    mat[matCell(0,2)] = -sin_teta;
    mat[matCell(2,0)] = sin_teta;
    mat[matCell(2,2)] = cos_teta;

    return 0;
}

static int mat_rot_z(double teta, transform_matrix mat)
{
    float cos_teta, sin_teta;

    teta *= Pi / 180.0;
    cos_teta = cos((double) teta);
    sin_teta = sin((double) teta);

    mat_unit(mat);                                  /* Make it unit matrix. */
    mat[matCell(0,0)] = cos_teta;
    mat[matCell(0,1)] = -sin_teta;
    mat[matCell(1,0)] = sin_teta;
    mat[matCell(1,1)] = cos_teta;

    return 0;
}

/* Multiply two transform_matrix. Result can be one of two operands. */
static void mat_mult(transform_matrix mat_res, transform_matrix mat1, 
	      transform_matrix mat2)
{
    int i, j, k;
    transform_matrix mat_res_temp;

    for (i = 0; i < 4; i++) for (j = 0; j < 4; j++) {
        mat_res_temp[matCell(i,j)] = 0;
        for (k = 0; k < 4; k++)
	   mat_res_temp[matCell(i,j)] += 
	      mat1[matCell(i,k)] * mat2[matCell(k,j)];
    }
    for (i = 0; i < 4; i++) for (j = 0; j < 4; j++)
	mat_res[matCell(i,j)] = mat_res_temp[matCell(i,j)];

}

/* And the functions to map from user 3D space to terminal coordinates */
int map3d_xy(double x, double y, double z, int *xt, int *yt)
{
    int i;
    float v[4], res[4],		/* Homogeneous coords. vectors. */
    w = trans_mat[15];		/* (3,3) */

    v[0] = map_x3d(x); /* Normalize object space to -1..1 */
    v[1] = map_y3d(y);
    v[2] = map_z3d(z);
    v[3] = 1.0;

    for (i = 0; i < 2; i++) {	             /* Dont use the third axes (z). */
        res[i] = trans_mat[12+i]             /* (3,i) */ /* weight factor */
	         + (v[0] * trans_mat[i])     /* (0,i) */
	         + (v[1] * trans_mat[4+i])   /* (1,i) */
	         + (v[2] * trans_mat[8+i]);  /* (2,i) */
    }

    w += (v[0] * trans_mat[3])      	/* (0,3) */
          + (v[1] * trans_mat[7])	/* (1,3) */
	  + (v[2] * trans_mat[11]);	/* (2,3) */

    if (w == 0) w = 1e-5;

    *xt = ((int) (res[0] * xscaler / w)) + xmiddle;
    *yt = ((int) (res[1] * yscaler / w)) + ymiddle;

  return 0;
}

/* Test a single point to be within the xleft,xright,ybot,ytop bbox.
 * Sets the returned integers 4 l.s.b. as follows:
 * bit 0 if to the left of xleft.
 * bit 1 if to the right of xright.
 * bit 2 if above of ytop.
 * bit 3 if below of ybot.
 * 0 is returned if inside.
 */
static int clip_point(int x, int y)
{
    int ret_val = 0;

    if (x < xleft) ret_val |= 0x01;
    if (x > xright) ret_val |= 0x02;
    if (y < ybot) ret_val |= 0x04;
    if (y > ytop) ret_val |= 0x08;

    return ret_val;
}

/* Clip the given line to drawing coords defined as xleft,xright,ybot,ytop.
 *   This routine uses the cohen & sutherland bit mapping for fast clipping -
 * see "Principles of Interactive Computer Graphics" Newman & Sproull page 65.
 */
static void draw_clip_line(int x1, int y1, int x2, int y2)
{
    int x, y, dx, dy, x_intr[2], y_intr[2], count, pos1, pos2;
    register struct termentry *t = &term_tbl[term];

    pos1 = clip_point(x1, y1);
    pos2 = clip_point(x2, y2);
    if (pos1 || pos2) {
	if (pos1 & pos2) return;		  /* segment is totally out. */

	/* Here part of the segment MAY be inside. test the intersection
	 * of this segment with the 4 boundaries for hopefully 2 intersections
	 * in. If non found segment is totaly out.
	 */
	count = 0;
	dx = x2 - x1;
	dy = y2 - y1;

	/* Find intersections with the x parallel bbox lines: */
	if (dy != 0) {
	    x = (ybot - y2) * dx / dy + x2;        /* Test for ybot boundary. */
	    if (x >= xleft && x <= xright) {
		x_intr[count] = x;
		y_intr[count++] = ybot;
	    }
	    x = (ytop - y2) * dx / dy + x2;        /* Test for ytop boundary. */
	    if (x >= xleft && x <= xright) {
		x_intr[count] = x;
		y_intr[count++] = ytop;
	    }
	}

	/* Find intersections with the y parallel bbox lines: */
	if (dx != 0) {
	    y = (xleft - x2) * dy / dx + y2;      /* Test for xleft boundary. */
	    if (y >= ybot && y <= ytop) {
		x_intr[count] = xleft;
		y_intr[count++] = y;
	    }
	    y = (xright - x2) * dy / dx + y2;    /* Test for xright boundary. */
	    if (y >= ybot && y <= ytop) {
		x_intr[count] = xright;
		y_intr[count++] = y;
	    }
	}

	if (count == 2) {
	    int x_max, x_min, y_max, y_min;

	    x_min = min(x1, x2);
	    x_max = max(x1, x2);
	    y_min = min(y1, y2);
	    y_max = max(y1, y2);

	    if (pos1 && pos2) {		       /* Both were out - update both */
		x1 = x_intr[0];
		y1 = y_intr[0];
		x2 = x_intr[1];
		y2 = y_intr[1];
	    }
	    else if (pos1) {	       /* Only x1/y1 was out - update only it */
		if (dx * (x2 - x_intr[0]) + dy * (y2 - y_intr[0]) > 0) {
		    x1 = x_intr[0];
		    y1 = y_intr[0];
		}
		else {
		    x1 = x_intr[1];
		    y1 = y_intr[1];
		}
	    }
	    else {	       	       /* Only x2/y2 was out - update only it */
		if (dx * (x_intr[0] - x1) + dy * (y_intr[0] - x1) > 0) {
		    x2 = x_intr[0];
		    y2 = y_intr[0];
		}
		else {
		    x2 = x_intr[1];
		    y2 = y_intr[1];
		}
	    }

	    if (x1 < x_min || x1 > x_max ||
		x2 < x_min || x2 > x_max ||
		y1 < y_min || y1 > y_max ||
		y2 < y_min || y2 > y_max) return;
	}
	else
	    return;
    }

    (*t->move)(x1,y1);
    (*t->vector)(x2,y2);
}


/* Two routine to emulate move/vector sequence using line drawing routine. */
static int move_pos_x, move_pos_y;

static void clip_move(int x,int y)
{
    move_pos_x = x;
    move_pos_y = y;
}

static void clip_vector(int x, int y)
{
    draw_clip_line(move_pos_x,move_pos_y, x, y);
    move_pos_x = x;
    move_pos_y = y;
}

/* And text clipping routine. */
static void clip_put_text(int x, int y, char *str)
{
    register struct termentry *t = &term_tbl[term];

    if (clip_point(x, y)) return;

    (*t->put_text)(x,y,str);
}


/* (DFK) For some reason, the Sun386i compiler screws up with the CheckLog 
 * macro, so I write it as a function on that machine.
 */
#ifndef sun386
/* (DFK) Use 10^x if logscale is in effect, else x */
#define CheckLog(log, x) ((log) ? pow(10., (x)) : (x))
#else
static float
CheckLog(log, x)
     BOOLEAN log;
     float x;
{
  if (log)
    return(pow(10., (double) x));
  else
    return(x);
}
#endif /* sun386 */


/* borders of plotting area */
/* computed once on every call to do_plot */
static int boundary3d(BOOLEAN scaling)
     /* BOOLEAN scaling:  TRUE if terminal is doing the scaling */
{
    register struct termentry *t = &term_tbl[term];
    xleft = (t->h_char)*2 + (t->h_tic);
    xright = (scaling ? 1 : xsize) * (t->xmax) - (t->h_char)*2 - (t->h_tic);
    ybot = (t->v_char)*3/2 + 1;
    ytop = (scaling ? 1 : ysize) * (t->ymax) - (t->v_char)*3/2 - 1;
    xmiddle = (xright + xleft) / 2;
    ymiddle = (ytop + ybot) / 2;
    xscaler = (xright - xleft) / 2;
    yscaler = (ytop - ybot) / 2;

return 0;
}

static float dbl_raise(double x, int y)
{
register int i;
float val;

	val = 1.0;
	for (i=0; i < abs(y); i++)
		val *= x;
	if (y < 0 ) return (1.0/val);
	return(val);
}


static float make_3dtics(double tmin, double tmax, int axis, BOOLEAN logscale)
{
int x1,y1,x2,y2;
register float xr,xnorm,tics,tic,l10;

	xr = fabs(tmin-tmax);

	/* Compute length of axis in screen space coords. */
	switch (axis) {
		case 'x':
			map3d_xy(tmin,0.0,0.0,&x1,&y1);
			map3d_xy(tmax,0.0,0.0,&x2,&y2);
			break;
		case 'y':
			map3d_xy(0.0,tmin,0.0,&x1,&y1);
			map3d_xy(0.0,tmax,0.0,&x2,&y2);
			break;
		case 'z':
			map3d_xy(0.0,0.0,tmin,&x1,&y1);
			map3d_xy(0.0,0.0,tmax,&x2,&y2);
			break;
	}

	if ((unsigned)((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)) <
	    sqr(3 * term_tbl[term].h_char))
		return -1.0;                  			/* No tics! */

	l10 = log10(xr);
	if (logscale) {
		tic = dbl_raise(10.0,(l10 >= 0.0 ) ? (int)l10 : ((int)l10-1));
		if (tic < 1.0)
			tic = 1.0;
	} else {
		xnorm = pow(10.0,l10-(double)((l10 >= 0.0 ) ? (int)l10 : ((int)l10-1)));
		if (xnorm <= 5)
			tics = 0.5;
		else tics = 1.0;
		tic = tics * dbl_raise(10.0,(l10 >= 0.0 ) ? (int)l10 : ((int)l10-1));
	}
	return(tic);
}

int do_3dplot(struct surface_points *plots, int pcount, double min_x, 
	      double max_x, double min_y, double max_y, double min_z, 
	      double max_z)
/* pcount: count of plots in linked list */
{
register struct termentry *t = &term_tbl[term];
register int surface;
register struct surface_points *this_plot;
register int xl=0, yl=0;
			/* only a Pyramid would have this many registers! */
float xtemp, ytemp, ztemp, temp;
struct text_label *this_label;
struct arrow_def *this_arrow;
BOOLEAN scaling;
transform_matrix mat;

/* Initiate transformation matrix using the global view variables. */
    mat_rot_z(surface_rot_z, trans_mat);
    mat_rot_x(surface_rot_x, mat);
    mat_mult(trans_mat, trans_mat, mat);
    mat_scale(surface_scale / 2.0, surface_scale / 2.0, surface_scale / 2.0, mat);
    mat_mult(trans_mat, trans_mat, mat);

/* modify min_z/max_z so it will zscale properly. */
    ztemp = (max_z - min_z) / (2.0 * surface_zscale);
    temp = (max_z + min_z) / 2.0;
    min_z = temp - ztemp;
    max_z = temp + ztemp;

/* store these in variables global to this file */
/* otherwise, we have to pass them around a lot */
    x_min3d = min_x;
    x_max3d = max_x;
    y_min3d = min_y;
    y_max3d = max_y;
    z_min3d = min_z;
    z_max3d = max_z;

    if (z_min3d == VERYLARGE || z_max3d == -VERYLARGE)
	terrain_error("all points undefined!");

    if (x_min3d == VERYLARGE || x_max3d == -VERYLARGE ||
	y_min3d == VERYLARGE || y_max3d == -VERYLARGE)
        terrain_error("all points undefined!");

    /* If we are to draw the bottom grid make sure zmin is updated properly. */
    if (xtics || ytics || grid)
	z_min3d -= (max_z - min_z) * ticslevel;

/*  This used be x_max3d == x_min3d, but that caused an infinite loop once. */
    if (fabs(x_max3d - x_min3d) < zero)
	terrain_error("x_min3d should not equal x_max3d!");
    if (fabs(y_max3d - y_min3d) < zero)
	terrain_error("y_min3d should not equal y_max3d!");
    if (fabs(z_max3d - z_min3d) < zero)
	terrain_error("z_min3d should not equal z_max3d!");

/* INITIALIZE TERMINAL */
    if (!term_init) {
	(*t->init)();
	term_init = TRUE;
    }
    scaling = (*t->scale)(xsize, ysize);
    (*t->graphics)();

    /* now compute boundary for plot (xleft, xright, ytop, ybot) */
    boundary3d(scaling);

/* SCALE FACTORS */
	zscale3d = 2.0/(z_max3d - z_min3d);
	yscale3d = 2.0/(y_max3d - y_min3d);
	xscale3d = 2.0/(x_max3d - x_min3d);

	(*t->linetype)(-2); /* border linetype */


/* PLACE TITLE */
	if (*title != 0) {
		int x, y;

		x = title_xoffset * t->h_char;
		y = title_yoffset * t->v_char - 160;

		if ((*t->justify_text)(CENTRE)) 
			(*t->put_text)(x+(xleft+xright)/2, 
				       y+ytop+(t->v_char), title);
		else
			(*t->put_text)(x+(xleft+xright)/2 - strlen(title)*(t->h_char)/2,
				       y+ytop+(t->v_char), title);
	}

/* PLACE LABELS */
    for (this_label = first_label; this_label!=NULL;
			this_label=this_label->next ) {
	    int x,y;

	/* place the curve labels in different place according to the rotational degree */
            if (surface_rot_z < 180 || surface_rot_z == 360)
               this_label->x = plots->x_max;
            else
               this_label->x = 0;

            if (surface_rot_z < 90 || (surface_rot_z > 180 && 
		surface_rot_z < 270) || surface_rot_z == 360)
	       this_label->pos = LEFT;
	    else
	       this_label->pos = RIGHT;

	    if (surface_rot_z < 90 && surface_rot_x > 90)
	    {
	       this_label->x = 0;
	       this_label->pos = RIGHT;
	    }
            else if (surface_rot_z > 90 && surface_rot_z < 180 && surface_rot_x > 90)
            {
	       this_label->x = 0;
	       this_label->pos = LEFT;
	    }
	    else if (surface_rot_z > 180 && surface_rot_z < 270 && surface_rot_x > 90)
	    {
	       this_label->x = plots->x_max;
	       this_label->pos = RIGHT;
	    }
	    else if (surface_rot_z > 270 && surface_rot_z < 360 && surface_rot_x > 90)
	    {
	       this_label->x = plots->x_max;
	       this_label->pos = LEFT;	
            }
	       

	    xtemp = this_label->x;
	    ytemp = this_label->y;
	    ztemp = this_label->z;
    	    map3d_xy(xtemp,ytemp,ztemp, &x, &y);

		if ((*t->justify_text)(this_label->pos)) {
			(*t->put_text)(x,y,this_label->text);
		}
		else {
		        if (surface_rot_x < 180)
			   this_label->pos = CENTRE;
			else
			   this_label->pos = CENTRE;

			switch(this_label->pos) {
				case  LEFT:
					(*t->put_text)(x,y,this_label->text);
					break;
				case CENTRE:
					(*t->put_text)(x -
						(t->h_char)*strlen(this_label->text)/2,
						y, this_label->text);
					break;
				case RIGHT:
					(*t->put_text)(x -
						(t->h_char)*strlen(this_label->text),
						y, this_label->text);
					break;
			}
		 }
	 }

/* PLACE ARROWS */
    (*t->linetype)(0);	/* arrow line type */
    for (this_arrow = first_arrow; this_arrow!=NULL;
	    this_arrow = this_arrow->next ) {
	int sx,sy,ex,ey;

	xtemp = this_arrow->sx;
	ytemp = this_arrow->sy;
	ztemp = this_arrow->sz;
	map3d_xy(xtemp,ytemp,ztemp, &sx, &sy);

	xtemp = this_arrow->ex;
	ytemp = this_arrow->ey;
	ztemp = this_arrow->ez;
	map3d_xy(xtemp,ytemp,ztemp, &ex, &ey);
	   
	(*t->arrow)(sx, sy, ex, ey);
    }


/* DRAW SURFACES AND CONTOURS */
	real_z_min3d = min_z;
	real_z_max3d = max_z;
	if (key == -1) {
	    xl = xright  - (t->h_tic) - (t->h_char)*5;
	    yl = ytop - (t->v_tic) - (t->v_char);
	}
	if (key == 1) {
	    xl = map_x(key_x);
	    yl = map_y(key_y);
	}

	this_plot = plots;
	for (surface = 0;
	     surface < pcount;
	     this_plot = this_plot->next_sp, surface++) {
		(*t->linetype)(this_plot->line_type);

		if ( surface == 0 )
			draw_bottom_grid(this_plot,real_z_min3d,real_z_max3d);


		if (!draw_surface) continue;
		(*t->linetype)(this_plot->line_type+1);

		if (key != 0) {
			if ((*t->justify_text)(RIGHT)) {
				clip_put_text(xl,
					yl,this_plot->title);
			}
			else {
			    if (inrange(xl-(t->h_char)*strlen(this_plot->title), 
						 (unsigned)xleft, (unsigned)xright))
				 clip_put_text(xl-(t->h_char)*strlen(this_plot->title),
							 yl,this_plot->title);
			}
		}
 
/*
		if (key != 0) {
		    clip_move(xl+(int)(t->h_char),yl);
		    clip_vector(xl+(int)(4*(t->h_char)),yl);
		}
*/

		plot3d_lines(this_plot, -1);

		yl = yl - (t->v_char);
	}
	(*t->text)();
	(void) fflush(outfile);

return 0;

}

/* plot3d_lines:
 * Plot the surfaces in LINES style
 */
int plot3d_lines(struct surface_points *plot, int printIndex)
{
    int i,j,k;				/* point index */
    
    static int path[4][4][2] = {   0,0,0,1,-1,1,-1,0,
				   0,0,0,1,1,1,1,0,
				   0,0,0,-1,1,-1,1,0,
				   0,0,0,-1,-1,-1,-1,0
                               };
    
    int i_st, i_end, i_step, j_st, j_end, j_step;
    int l, path_i;
    float x[4],y[4],z[4];
    int x2d[4], y2d[4]; 

/***********************************************************************************
 ************************* modified section starts *********************************/

    int index1, index2;
    struct coordinate *points;
    int numSample;
    int endPoints;
    int remap = printIndex;

    /***********************************************************************/
    /* printIndex = 0        draw nothing                                  */
    /* printIndex = -1       redraw all points up to the last print point  */
    /* printIndex > 0        draw that specific points with connection with*/
    /*                         the last print point                        */
    /***********************************************************************/


    if (printIndex == 0)
       return 0;

    else if (printIndex == -1)
    {
       points = (struct coordinate*) (terrain_alloc(sizeof(struct coordinate) *
                ((plot->lastprintIndex + 1) * plot->iso_samples * 2)));

       numSample = (plot->lastprintIndex + 1) * plot->iso_samples;
       endPoints = plot->lastprintIndex + 1;

       for (index1 = 0;  index1 < endPoints; index1 ++)
       {
           for (index2 = 0; index2 < plot->iso_samples; index2 ++)
	   {
	       points[index1 + index2 * endPoints].x = 
		    plot->points[index1 + index2 * plot->samples].x;
    	       points[index1 + index2 * endPoints].y =
                    plot->points[index1 + index2 * plot->samples].y;
	       points[index1 + index2 * endPoints].z =
                    plot->points[index1 + index2 * plot->samples].z;

	       points[index1 + index2 * endPoints + numSample].x = 
		    plot->points[index1 + index2 * plot->samples].x;
    	       points[index1 + index2 * endPoints + numSample].y =
                    plot->points[index1 + index2 * plot->samples].y;
	       points[index1 + index2 * endPoints + numSample].z =
                    plot->points[index1 + index2 * plot->samples].z;


	      
	   }
          
       }     
    }

    else
    {
       points = (struct coordinate*) (terrain_alloc(sizeof(struct coordinate) *
	        (plot->iso_samples * 4)));

       numSample = plot->iso_samples * 2;
       endPoints = 2;
       for (index1 = 0; index1 < plot->iso_samples * 2; index1 +=2)
       {
	   points[index1].x = plot->points[plot->samples * (index1 / 2) + printIndex - 1].x;
	   points[index1].y = plot->points[plot->samples * (index1 / 2) + printIndex - 1].y;
	   points[index1].z = plot->points[plot->samples * (index1 / 2) + printIndex - 1].z;

	   points[index1 + 1].x = plot->points[plot->samples * (index1 / 2) + printIndex].x;
	   points[index1 + 1].y = plot->points[plot->samples * (index1 / 2) + printIndex].y;
	   points[index1 + 1].z = plot->points[plot->samples * (index1 / 2) + printIndex].z;
       }

    }


  

/**************************modified section ends ************************************
 ************************************************************************************/

    if (surface_rot_z < 90) {
	path_i = 0;
	j_st = plot->iso_samples-1;
	j_end = 0;
	i_st = 0;
	i_end = endPoints - 1;
    } else if (surface_rot_z < 180) {
	path_i = 1;
	j_st = 0;
	j_end = plot->iso_samples-1;
	i_st = 0;
	i_end = endPoints - 1;
    } else if (surface_rot_z < 270) {
	path_i = 2;
	j_st = 0;
	j_end = plot->iso_samples-1;
	i_st = endPoints - 1;
	i_end = 0;
    } else {
	path_i = 3;
	j_st = plot->iso_samples-1;
	j_end = 0;
	i_st = endPoints - 1;
	i_end = 0;
    }
    
    i_step = 1 - 2*(i_st > i_end);
    j_step = 1 - 2*(j_st > j_end);
    
    for (j = j_st; j != j_end; j+=j_step) {
	for (i = i_st; i != i_end; i+=i_step) {
	    for(l=0; l<4; l++) {
		
		k = (j+path[path_i][l][0]) * endPoints +
		    (i+path[path_i][l][1]);
		
		if (real_z_max3d<points[k].z)
		   real_z_max3d=points[k].z;
		if (real_z_min3d>points[k].z)
		   real_z_min3d=points[k].z;
		
		map3d_xy(points[k].x, points[k].y,
			 points[k].z, &(x2d[l]), &(y2d[l]));
		
		x[l]=points[k].x;
		y[l]=points[k].y;
		z[l]=points[k].z;
	    }
	    
#ifdef COLORIZE_DEBUG
	    printf("\n------ j=%d ----- i=%d ------------\n",j ,i);
#endif 
	    if (color_mode && color_disp) {
		draw_colorPoly(x[0],y[0],z[0],
			       x[1],y[1],z[1],
			       x[2],y[2],z[2],
			       x[3],y[3],z[3],
			       remap, printIndex);
		remap = 0;


		   if (printIndex > 0)
		       X11_poly_window(x2d[0],y2d[0],x2d[1],y2d[1],x2d[2],y2d[2],
			  x2d[3],y2d[3]);

		   X11_poly(x2d[0],y2d[0],x2d[1],y2d[1],x2d[2],y2d[2],
			  x2d[3],y2d[3]);

	    } else {
	           if (printIndex > 0)
		      X11_monoPoly_window(x2d[0],y2d[0],x2d[1],y2d[1],x2d[2],y2d[2],
			      x2d[3],y2d[3]);

		   X11_monoPoly(x2d[0],y2d[0],x2d[1],y2d[1],x2d[2],y2d[2],
			      x2d[3],y2d[3]);
	  }

	}
    }

    
    free (points);

return 0;

}

int update_extrema_pts(int ix, int iy, int *min_sx_x, int *min_sx_y, 
			      int *min_sy_x, int *min_sy_y,
			      double x, double y)
{

    if (*min_sx_x > ix + 2 ||         /* find (bottom) left corner of grid */
	(abs(*min_sx_x - ix) <= 2 && *min_sx_y > iy)) {
	*min_sx_x = ix;
	*min_sx_y = iy;
	min_sx_ox = x;
	min_sx_oy = y;
    }
    if (*min_sy_y > iy + 2 ||         /* find bottom (right) corner of grid */
	(abs(*min_sy_y - iy) <= 2 && *min_sy_x < ix)) {
	*min_sy_x = ix;
	*min_sy_y = iy;
	min_sy_ox = x;
	min_sy_oy = y;
    }

    return 0;
}

/* Draw the bottom grid for the parametric case. */
static int draw_parametric_grid(double z_min)
{
    int i,ix,iy,			/* point in terminal coordinates */
	min_sx_x = 10000,min_sx_y = 10000,min_sy_x = 10000,min_sy_y = 10000;
    float x,y,dx,dy;
    struct termentry *t = &term_tbl[term];

    if (grid) {
    	x = x_min3d;
    	y = y_min3d;

	dx = (x_max3d-x_min3d) / (iso_samples-1);
	dy = (x_max3d-x_min3d) / (iso_samples-1);

	for (i = 0; i < iso_samples; i++) {
	        if (i == 0 || i == iso_samples-1)
		    (*t->linetype)(-2);
		else
		    (*t->linetype)(-1);
		map3d_xy(x_min3d, y, z_min, &ix, &iy);
		clip_move(ix,iy);
		update_extrema_pts(ix,iy,&min_sx_x,&min_sx_y,
				   &min_sy_x,&min_sy_y,x_min3d,y);

		map3d_xy(x_max3d, y, z_min, &ix, &iy);
		clip_vector(ix,iy);
		update_extrema_pts(ix,iy,&min_sx_x,&min_sx_y,
				   &min_sy_x,&min_sy_y,x_max3d,y);

		y += dy;
	}

	for (i = 0; i < iso_samples; i++) {
	        if (i == 0 || i == iso_samples-1)
		    (*t->linetype)(-2);
		else
		    (*t->linetype)(-1);
		map3d_xy(x, y_min3d, z_min, &ix, &iy);
		clip_move(ix,iy);
		update_extrema_pts(ix,iy,&min_sx_x,&min_sx_y,
				   &min_sy_x,&min_sy_y,x,y_min3d);

		map3d_xy(x, y_max3d, z_min, &ix, &iy);
		clip_vector(ix,iy);
		update_extrema_pts(ix,iy,&min_sx_x,&min_sx_y,
				   &min_sy_x,&min_sy_y,x,y_max3d);

		x += dx;
	}
    }
    else {
	(*t->linetype)(-2);

	map3d_xy(x_min3d, y_min3d, z_min, &ix, &iy);
	clip_move(ix,iy);
	update_extrema_pts(ix,iy,&min_sx_x,&min_sx_y,
			   &min_sy_x,&min_sy_y,x_min3d,y_min3d);

	map3d_xy(x_max3d, y_min3d, z_min, &ix, &iy);
	clip_vector(ix,iy);
	update_extrema_pts(ix,iy,&min_sx_x,&min_sx_y,
			   &min_sy_x,&min_sy_y,x_max3d,y_min3d);

	map3d_xy(x_max3d, y_max3d, z_min, &ix, &iy);
	clip_vector(ix,iy);
	update_extrema_pts(ix,iy,&min_sx_x,&min_sx_y,
			   &min_sy_x,&min_sy_y,x_max3d,y_max3d);

	map3d_xy(x_min3d, y_max3d, z_min, &ix, &iy);
	clip_vector(ix,iy);
	update_extrema_pts(ix,iy,&min_sx_x,&min_sx_y,
			   &min_sy_x,&min_sy_y,x_min3d,y_max3d);


	map3d_xy(x_min3d, y_min3d, z_min, &ix, &iy);
	clip_vector(ix,iy);
    }
   return 0;
}


/* Draw the bottom grid that hold the tic marks for 3d surface. */
static int draw_bottom_grid(struct surface_points *plot, double min_z, 
			    double max_z)
{
    int x,y;
    float xtic,ytic,ztic;
    struct termentry *t = &term_tbl[term];

    xtic = make_3dtics(x_min3d,x_max3d,'x',log_x);
    ytic = make_3dtics(y_min3d,y_max3d,'y',log_y);
    ztic = make_3dtics(min_z,max_z,'z',log_z);

    if (draw_border) draw_parametric_grid(min_z);

    (*t->linetype)(-2); /* border linetype */

/* label x axis tics */
    if (xtics && xtic > 0.0) {
    	switch (xticdef.type) {
    	    case TIC_COMPUTED:
 		if (x_min3d < x_max3d)
		    draw_3dxtics(xtic * floor(x_min3d/xtic),
    				 xtic,
    				 xtic * ceil(x_max3d/xtic),
    				 min_sy_oy, min_z);
    	    	else
		    draw_3dxtics(xtic * floor(x_max3d/xtic),
    				 xtic,
    				 xtic * ceil(x_min3d/xtic),
    				 min_sy_oy, min_z);
    		break;
	    case TIC_SERIES:
		draw_series_3dxtics(xticdef.def.series.start, 
				    xticdef.def.series.incr, 
				    xticdef.def.series.end,
				    min_sy_oy, min_z);
		break;
	    case TIC_USER:
		draw_set_3dxtics(xticdef.def.user,
				 min_sy_oy, min_z);
		break;
    	    default:
    		(*t->text)();
    		(void) fflush(outfile);
    		terrain_error("unknown tic type in xticdef in do_3dplot");
    		break;		/* NOTREACHED */
    	}
    }



/* label y axis tics */
/************************************************************************************
 ******************** not going to use in dynamic graph *****************************

    if (ytics && ytic > 0.0) {
    	switch (yticdef.type) {
    	    case TIC_COMPUTED:
 		if (y_min3d < y_max3d)
		    draw_3dytics(ytic * floor(y_min3d/ytic),
    				 ytic,
    				 ytic * ceil(y_max3d/ytic),
    				 min_sy_ox, min_z);
    	    	else
		    draw_3dytics(ytic * floor(y_max3d/ytic),
    				 ytic,
    				 ytic * ceil(y_min3d/ytic),
    				 min_sy_ox, min_z);
    		break;
	    case TIC_SERIES:
		draw_series_3dytics(yticdef.def.series.start, 
				    yticdef.def.series.incr, 
				    yticdef.def.series.end,
				    min_sy_ox, min_z);
		break;
	    case TIC_USER:
		draw_set_3dytics(yticdef.def.user,
				 min_sy_ox, min_z);
		break;
    	    default:
    		(*t->text)();
    		(void) fflush(outfile);
    		terrain_error("unknown tic type in yticdef in do_3dplot");
    		break;		 NOTREACHED
    	}
    }

 ************************************************************************************
 ************************************************************************************/



/* label z axis tics */
    if (ztics && ztic > 0.0 && (draw_surface ||
				draw_contour == CONTOUR_SRF ||
				draw_contour == CONTOUR_BOTH)) {
    	switch (zticdef.type) {
    	    case TIC_COMPUTED:
 		if (min_z < max_z)
		    draw_3dztics(ztic * floor(min_z/ztic),
    				 ztic,
    				 ztic * ceil(max_z/ztic),
				 min_sx_ox,
    				 min_sx_oy,
    				 min_z,
				 max_z);
    	    	else
		    draw_3dztics(ztic * floor(max_z/ztic),
    				 ztic,
    				 ztic * ceil(min_z/ztic),
    				 min_sx_ox,
				 min_sx_oy,
    				 max_z,
				 min_z);
    		break;
	    case TIC_SERIES:
		draw_series_3dztics(zticdef.def.series.start, 
				    zticdef.def.series.incr, 
				    zticdef.def.series.end,
				    min_sx_ox,
				    min_sx_oy,
				    min_z,
				    max_z);

		break;
	    case TIC_USER:
		draw_set_3dztics(zticdef.def.user,
				 min_sx_ox,
    				 min_sx_oy,
    				 min_z,
				 max_z);
		break;
    	    default:
    		(*t->text)();
    		(void) fflush(outfile);
    		terrain_error("unknown tic type in zticdef in do_3dplot");
    		break;		/* NOTREACHED */
    	}
    }

/* PLACE XLABEL - along the middle grid X axis */
    if (xlabel != NULL) {
	   int x1,y1;
	   float step = apx_eq( min_sy_oy, y_min3d ) ?	(y_max3d-y_min3d)/4
						      : (y_min3d-y_max3d)/4;
    	   map3d_xy((x_min3d+x_max3d)/2,min_sy_oy-step, z_min3d ,&x1,&y1);
	   x1 += xlabel_xoffset * t->h_char;
	   y1 += xlabel_yoffset * t->v_char;
	   if ((*t->justify_text)(CENTRE))
		clip_put_text(x1,y1,xlabel);
	   else
		clip_put_text(x1 - strlen(xlabel)*(t->h_char)/2,y1,xlabel);
    }

/* PLACE YLABEL - along the middle grid Y axis */
    if (ylabel != NULL) {
	   int x1,y1;
	   float step = apx_eq( min_sy_ox, x_min3d ) ?	(x_max3d-x_min3d)/4
						      : (x_min3d-x_max3d)/4;
    	   map3d_xy(min_sy_ox-step,(y_min3d+y_max3d)/2,z_min3d ,&x1,&y1);
	   x1 += ylabel_xoffset * t->h_char;
	   y1 += ylabel_yoffset * t->v_char;
	   if ((*t->justify_text)(CENTRE))
		clip_put_text(x1,y1,ylabel);
	   else
		clip_put_text(x1 - strlen(ylabel)*(t->h_char)/2,y1,ylabel);
    }

/* PLACE ZLABEL - along the middle grid Z axis */
    if (zlabel != NULL) {
    	   map3d_xy(min_sx_ox,min_sx_oy,max_z + (max_z-min_z)/4, &x, &y);

	   x += zlabel_xoffset * t->h_char;
	   y += zlabel_yoffset * t->v_char;
	   if ((*t->justify_text)(CENTRE))
		clip_put_text(x,y,zlabel);
	   else
		clip_put_text(x - strlen(zlabel)*(t->h_char)/2,y,zlabel);
    }

return 0;

}

/* DRAW_3DXTICS: draw a regular tic series, x axis */
int draw_3dxtics(double start, double incr, double end, double ypos, 
		 double z_min)
     /* start, incr, end, ypos: tic series definition */
     /* assume start < end, incr > 0 */
{
	float ticplace;
	int ltic;		/* for mini log tics */
	float lticplace;	/* for mini log tics */

	end = end + SIGNIF*incr; 

	for (ticplace = start; ticplace <= end; ticplace +=incr) {
		if (ticplace < x_min3d || ticplace > x_max3d) continue;
		xtick(ticplace, xformat, incr, 1.0, ypos, z_min);
		if (log_x && incr == 1.0) {
			/* add mini-ticks to log scale ticmarks */
			for (ltic = 2; ltic <= 9; ltic++) {
				lticplace = ticplace+log10((double)ltic);
				xtick(lticplace, "\0", incr, 0.5, ypos, z_min);
			}
		}
	}

return 0;
}

/* DRAW_3DYTICS: draw a regular tic series, y axis */
int draw_3dytics(double start, double incr, double end, double xpos, 
		 double z_min)
     /* start, incr, end, xpos: tic series definition */
     /* assume start < end, incr > 0 */
{
	float ticplace;
	int ltic;		/* for mini log tics */
	float lticplace;	/* for mini log tics */

	end = end + SIGNIF*incr; 

	for (ticplace = start; ticplace <= end; ticplace +=incr) {
		if (ticplace < y_min3d || ticplace > y_max3d) continue;
		ytick(ticplace, yformat, incr, 1.0, xpos, z_min);
		if (log_y && incr == 1.0) {
			/* add mini-ticks to log scale ticmarks */
			for (ltic = 2; ltic <= 9; ltic++) {
				lticplace = ticplace+log10((double)ltic);
				ytick(lticplace, "\0", incr, 0.5, xpos, z_min);
			}
		}
	}
return 0;

}

/* DRAW_3DZTICS: draw a regular tic series, z axis */
int draw_3dztics(double start, double incr, double end, double xpos, 
		 double ypos, double z_min, double z_max)
		/* assume start < end, incr > 0 */
{
	int x, y;
	float ticplace;
	int ltic;		/* for mini log tics */
	float lticplace;	/* for mini log tics */
	register struct termentry *t = &term_tbl[term];

	end = end + SIGNIF*incr; 

	for (ticplace = start; ticplace <= end; ticplace +=incr) {
		if (ticplace < z_min || ticplace > z_max) continue;

		ztick(ticplace, zformat, incr, 1.0, xpos, ypos);
		if (log_z && incr == 1.0) {
			/* add mini-ticks to log scale ticmarks */
			for (ltic = 2; ltic <= 9; ltic++) {
				lticplace = ticplace+log10((double)ltic);
				ztick(lticplace, "\0", incr, 0.5, xpos, ypos);
			}
		}
	}

	/* Make sure the vertical line is fully drawn. */
	(*t->linetype)(-2);	/* axis line type */

	map3d_xy(xpos, ypos, z_min, &x, &y);
	clip_move(x,y);
	map3d_xy(xpos, ypos, min(end,z_max)+(log_z ? incr : 0.0), &x, &y);
	clip_vector(x,y);

	(*t->linetype)(-1); /* border linetype */

 return 0;
}

/* DRAW_SERIES_3DXTICS: draw a user tic series, x axis */
int draw_series_3dxtics(double start, double incr, double end, double ypos, 
			double z_min)
     /* start, incr, end, ypos: tic series definition */
     /* assume start < end, incr > 0 */
{
	float ticplace, place;
	float ticmin, ticmax;	/* for checking if tic is almost inrange */
	float spacing = log_x ? log10(incr) : incr;

	if (end == VERYLARGE)
		end = max(CheckLog(log_x, x_min3d), CheckLog(log_x, x_max3d));
	else
	  /* limit to right side of plot */
	  end = min(end, max(CheckLog(log_x, x_min3d), CheckLog(log_x, x_max3d)));

	/* to allow for rounding errors */
	ticmin = min(x_min3d,x_max3d) - SIGNIF*incr;
	ticmax = max(x_min3d,x_max3d) + SIGNIF*incr;
	end = end + SIGNIF*incr; 

	for (ticplace = start; ticplace <= end; ticplace +=incr) {
	    place = (log_x ? log10(ticplace) : ticplace);
	    if ( inrange(place,ticmin,ticmax) )
		 xtick(place, xformat, spacing, 1.0, ypos, z_min);
	}

return 0;
}

/* DRAW_SERIES_3DYTICS: draw a user tic series, y axis */
int draw_series_3dytics(double start, double incr, double end, double xpos, 
			double z_min)
     /* start, incr, end, xpos: tic series definition */
     /* assume start < end, incr > 0 */
{
	float ticplace, place;
	float ticmin, ticmax;	/* for checking if tic is almost inrange */
	float spacing = log_y ? log10(incr) : incr;

	if (end == VERYLARGE)
		end = max(CheckLog(log_y, y_min3d), CheckLog(log_y, y_max3d));
	else
	  /* limit to right side of plot */
	  end = min(end, max(CheckLog(log_y, y_min3d), CheckLog(log_y, y_max3d)));

	/* to allow for rounding errors */
	ticmin = min(y_min3d,y_max3d) - SIGNIF*incr;
	ticmax = max(y_min3d,y_max3d) + SIGNIF*incr;
	end = end + SIGNIF*incr; 

	for (ticplace = start; ticplace <= end; ticplace +=incr) {
	    place = (log_y ? log10(ticplace) : ticplace);
	    if ( inrange(place,ticmin,ticmax) )
		 ytick(place, xformat, spacing, 1.0, xpos, z_min);
	}
return 0;

}

/* DRAW_SERIES_3DZTICS: draw a user tic series, z axis */
int draw_series_3dztics(double start, double incr, double end, double xpos, 
			double ypos, double z_min, double z_max)
     /* start, incr, end: tic series definition */
     /* assume start < end, incr > 0 */
{
	int x, y;
	float ticplace, place;
	float ticmin, ticmax;	/* for checking if tic is almost inrange */
	float spacing = log_x ? log10(incr) : incr;
	register struct termentry *t = &term_tbl[term];

	if (end == VERYLARGE)
		end = max(CheckLog(log_z, z_min), CheckLog(log_z, z_max));
	else
	  /* limit to right side of plot */
	  end = min(end, max(CheckLog(log_z, z_min), CheckLog(log_z, z_max)));

	/* to allow for rounding errors */
	ticmin = min(z_min,z_max) - SIGNIF*incr;
	ticmax = max(z_min,z_max) + SIGNIF*incr;
	end = end + SIGNIF*incr; 

	for (ticplace = start; ticplace <= end; ticplace +=incr) {
	    place = (log_z ? log10(ticplace) : ticplace);
	    if ( inrange(place,ticmin,ticmax) )
		 ztick(place, zformat, spacing, 1.0, xpos, ypos);
	}

	/* Make sure the vertical line is fully drawn. */
	(*t->linetype)(-2);	/* axis line type */

	map3d_xy(xpos, ypos, max(start,z_min), &x, &y);
	clip_move(x,y);
	map3d_xy(xpos, ypos, min(end,z_max)+(log_z ? incr : 0.0), &x, &y);
	clip_vector(x,y);

	(*t->linetype)(-1); /* border linetype */

return 0;
}

/* DRAW_SET_3DXTICS: draw a user tic set, x axis */
int draw_set_3dxtics(struct ticmark *list, double ypos, double z_min)
     /* struct ticmark *list: list of tic marks */
{
    float ticplace;
    float incr = (x_max3d - x_min3d) / 10;
    /* global x_min3d, x_max3d, xscale, y_min3d, y_max3d, yscale */

    while (list != NULL) {
	   ticplace = (log_x ? log10(list->position) : list->position);
	   if ( inrange(ticplace, x_min3d, x_max3d) 		/* in range */
		  || NearlyEqual(ticplace, x_min3d, incr)	/* == x_min */
		  || NearlyEqual(ticplace, x_max3d, incr))	/* == x_max */
		xtick(ticplace, list->label, incr, 1.0, ypos, z_min);

	   list = list->next;
    }

return 0;
}

/* DRAW_SET_3DYTICS: draw a user tic set, y axis */
int draw_set_3dytics(struct ticmark *list, double xpos, double z_min)
{
    float ticplace;
    float incr = (y_max3d - y_min3d) / 10;
    /* global x_min3d, x_max3d, xscale, y_min3d, y_max3d, yscale */

    while (list != NULL) {
	   ticplace = (log_y ? log10(list->position) : list->position);
	   if ( inrange(ticplace, y_min3d, y_max3d) 		  /* in range */
		  || NearlyEqual(ticplace, y_min3d, incr)	/* == y_min3d */
		  || NearlyEqual(ticplace, y_max3d, incr))	/* == y_max3d */
		ytick(ticplace, list->label, incr, 1.0, xpos, z_min);

	   list = list->next;
    }
    return 0;
}

/* DRAW_SET_3DZTICS: draw a user tic set, z axis */
int draw_set_3dztics(struct ticmark *list, double xpos, double ypos, 
		     double z_min, double z_max)
     /* struct ticmark *list:  list of tic marks */
{
    int x, y;
    float ticplace;
    float incr = (z_max - z_min) / 10;
    register struct termentry *t = &term_tbl[term];

    while (list != NULL) {
	   ticplace = (log_z ? log10(list->position) : list->position);
	   if ( inrange(ticplace, z_min, z_max) 		/* in range */
		  || NearlyEqual(ticplace, z_min, incr)		/* == z_min */
		  || NearlyEqual(ticplace, z_max, incr))	/* == z_max */
		ztick(ticplace, list->label, incr, 1.0, xpos, ypos);

	   list = list->next;
    }

    /* Make sure the vertical line is fully drawn. */
    (*t->linetype)(-2);	/* axis line type */

    map3d_xy(xpos, ypos, z_min, &x, &y);
    clip_move(x,y);
    map3d_xy(xpos, ypos, z_max+(log_z ? incr : 0.0), &x, &y);
    clip_vector(x,y);

    (*t->linetype)(-1); /* border linetype */

return 0;
}

/* draw and label a x-axis ticmark */
int xtick(double place, char *text, double spacing, double ticscale, 
	  double ypos, double z_min)
     /* place:     where on axis to put it */
     /* text:      optional text label */
     /* spacing:   something to use with checkzero */
     /* ticscale:  scale factor for tic mark (0..1] */
{
    register struct termentry *t = &term_tbl[term];
    char ticlabel[101];
    int x0,y0,x1,y1,x2,y2,x3,y3;
    int ticsize = (int)((t->h_tic) * ticscale);
    float v[2], len;
    int hour, min, sec;

    place = CheckZero(place,spacing); /* to fix rounding error near zero */

    if (place > x_max3d || place < x_min3d) return;

    map3d_xy(place, ypos,(3*z_min + z_min3d)/4 , &x0, &y0);
    /* need to figure out which is in. pick the middle point along the */
    /* axis as in.						       */
    map3d_xy(place, (y_max3d + y_min3d) / 2, (3*z_min + z_min3d)/4, &x1, &y1);

    /* compute a vector of length 1 into the grid: */
    v[0] = x1 - x0;
    v[1] = y1 - y0;
    len = sqrt(v[0] * v[0] + v[1] * v[1]);

    /* Take care special case which will cause floating point exceptions (chi)*/

    if (len == 0.0) {
	v[0] = 0; v[1] = 0;
    } else {
       v[0] /= len;
       v[1] /= len;
    }

    if (tic_in) {
	x1 = x0;
	y1 = y0;
	x2 = x1 + ((int) (v[0] * ticsize * 2));
	y2 = y1 + ((int) (v[1] * ticsize * 2));
    	x3 = x0 - ((int) (v[0] * ticsize * 2)); /* compute text position */
    	y3 = y0 - ((int) (v[1] * ticsize * 2));
    } else {
	x1 = x0 - ((int) (v[0] * ticsize));
	y1 = y0 - ((int) (v[1] * ticsize));
	x2 = x0 + ((int) (v[0] * ticsize));
	y2 = y0 + ((int) (v[1] * ticsize));
    	x3 = x0 - ((int) (v[0] * ticsize * 3)); /* compute text position */
    	y3 = y0 - ((int) (v[1] * ticsize * 3));
    }

/* Don't draw the tic for now 

    clip_move(x1,y1);
    clip_vector(x2,y2);

*/

    /* label the ticmark */
    if (text == NULL)
	 text = xformat;

    sec = 0; 
    min = 0;
    hour = 0;



      sec = (int)place % 60;
      min = place / 60;
    
      if (min > 60)
      {
         hour = min / 60;
         min %= 60;
      }

    if (timeAxis == 0)
    {
       (void) sprintf(ticlabel, "%d%s%.2d", min, ":", sec);
       strcpy(xlabel, "time (min:sec)");
    }
    else
    {
       (void) sprintf(ticlabel, "%d%s%.2d", hour, ":", min);
       strcpy(xlabel, "time (hour:min)");
    }
       
    if (apx_eq(v[0], 0.0)) {
    	if ((*t->justify_text)(CENTRE)) {
    	    clip_put_text(x3,y3,ticlabel);
    	} else {
    	    clip_put_text(x3-(t->h_char)*strlen(ticlabel)/2,y3,ticlabel);
    	}
    }
    else if (v[0] > 0) {
    	if ((*t->justify_text)(RIGHT)) {
    	    clip_put_text(x3,y3,ticlabel);
    	} else {
    	    clip_put_text(x3-(t->h_char)*strlen(ticlabel),y3,ticlabel);
    	}
    } else {
    	(*t->justify_text)(LEFT);
	clip_put_text(x3,y3,ticlabel);
    }
  return 0;

}

/* draw and label a y-axis ticmark */
int ytick(double place, char *text, double spacing, double ticscale, 
	  double xpos, double z_min)
     /* place:    where on axis to put it */
     /* text:     optional text label */
     /* spacing:  something to use with checkzero */
     /* ticscale: scale factor for tic mark (0..1] */
{
    register struct termentry *t = &term_tbl[term];
    char ticlabel[101];
    int x0,y0,x1,y1,x2,y2,x3,y3;
    int ticsize = (int)((t->h_tic) * ticscale);
    float v[2], len;

    place = CheckZero(place,spacing); /* to fix rounding error near zero */

    if (place > y_max3d || place < y_min3d) return;

    map3d_xy(xpos, place, (3*z_min + z_min3d)/4 , &x0, &y0);
    /* need to figure out which is in. pick the middle point along the */
    /* axis as in.						       */
    map3d_xy((x_max3d + x_min3d) / 2, place, (3*z_min + z_min3d)/4 , &x1, &y1);

    /* compute a vector of length 1 into the grid: */
    v[0] = x1 - x0;
    v[1] = y1 - y0;
    len = sqrt(v[0] * v[0] + v[1] * v[1]);

    /* Take care special case which will cause floating point exceptions (chi)*/

    if (len == 0.0) {
	v[0] = 0; v[1] = 0;
    } else {
       v[0] /= len;
       v[1] /= len;
    }

    if (tic_in) {
	x1 = x0;
	y1 = y0;
	x2 = x1 + ((int) (v[0] * ticsize * 2));
	y2 = y1 + ((int) (v[1] * ticsize * 2));
    	x3 = x0 - ((int) (v[0] * ticsize * 2)); /* compute text position */
    	y3 = y0 - ((int) (v[1] * ticsize * 2));

    } else {
	x1 = x0 - ((int) (v[0] * ticsize));
	y1 = y0 - ((int) (v[1] * ticsize));
	x2 = x0 + ((int) (v[0] * ticsize));
	y2 = y0 + ((int) (v[1] * ticsize));
    	x3 = x0 - ((int) (v[0] * ticsize * 3)); /* compute text position */
    	y3 = y0 - ((int) (v[1] * ticsize * 3));
    }

/* Don't draw the tick for now

    clip_move(x1,y1);
    clip_vector(x2,y2);

*/

    /* label the ticmark */
    if (text == NULL)
	 text = yformat;

    (void) sprintf(ticlabel, text, CheckLog(log_y, place));
    if (apx_eq(v[0], 0.0)) {
    	if ((*t->justify_text)(CENTRE)) {
    	    clip_put_text(x3,y3,ticlabel);
    	} else {
    	    clip_put_text(x3-(t->h_char)*strlen(ticlabel)/2,y3,ticlabel);
    	}
    }
    else if (v[0] > 0) {
    	if ((*t->justify_text)(RIGHT)) {
    	    clip_put_text(x3,y3,ticlabel);
    	} else {
    	    clip_put_text(x3-(t->h_char)*strlen(ticlabel),y3,ticlabel);
    	}
    } else {
    	(*t->justify_text)(LEFT);
	clip_put_text(x3,y3,ticlabel);
    }

return 0;
}

/* draw and label a z-axis ticmark */
int ztick(double place, char *text, double spacing, double ticscale, 
	  double xpos, double ypos)
     /* place:      where on axis to put it */
     /* text:       optional text label */
     /* spacing:    something to use with checkzero */
     /* ticscale:   scale factor for tic mark (0..1] */
{
    register struct termentry *t = &term_tbl[term];
    char ticlabel[101];
    int x0,y0,x1,y1,x2,y2,x3,y3;
    int ticsize = (int)((t->h_tic) * ticscale);
    int putzeroIndex;

    place = CheckZero(place,spacing); /* to fix rounding error near zero */

    map3d_xy(xpos, ypos, place, &x0, &y0);

    if (tic_in) {
	x1 = x0;
	y1 = y0;
	x2 = x0 + ticsize * 2;
	y2 = y0;
    	x3 = x0 - ticsize;
    	y3 = y0;

    } else {
	x1 = x0 - ticsize;
	y1 = y0;
	x2 = x0 + ticsize;
	y2 = y0;
    	x3 = x0 - ticsize * 2; /* compute text position */
    	y3 = y0;
    }
    clip_move(x1,y1);
    clip_vector(x2,y2);

    /* label the ticmark */
    if (text == NULL)
	 text = zformat;

    if (place != 0)
       (void) sprintf(ticlabel, text, CheckLog(log_z, place));
    else
       (void) sprintf(ticlabel, "%g", CheckLog(log_z, place));


    if ((*t->justify_text)(RIGHT)) {
        clip_put_text(x3,y3,ticlabel);
    } else {
        clip_put_text(x3-(t->h_char)*(strlen(ticlabel)+1),y3,ticlabel);
    }

return 0;
}



