/* Gnuplot - setshow.c */
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
 * send your comments or suggestions to (pixar!info-gnuplot@sun.com).
 * 
 */

/*
 * Adapted for terrain plot:
 *     Chi-Ting Lam
 *
 * setshow.c - option settings for GNUPlot.
 *
 * $Id: setshow.c,v 1.4 1998/03/30 01:22:33 wylie Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "plot.h"
#include "setshow.h"
#include "misc.h"

#define DEF_FORMAT   "%g"	/* default format for tic mark labels */

#define SIGNIF (0.01)		/* less than one hundredth of a tic mark */

/*
 * global variables to hold status of 'set' options
 *
 */
BOOLEAN			autoscale_t	= TRUE;
BOOLEAN			autoscale_u	= TRUE;
BOOLEAN			autoscale_v	= TRUE;
BOOLEAN			autoscale_x	= TRUE;
BOOLEAN			autoscale_y	= TRUE;
BOOLEAN			autoscale_z	= TRUE;
BOOLEAN			autoscale_lt	= TRUE;
BOOLEAN			autoscale_lu	= TRUE;
BOOLEAN			autoscale_lv	= TRUE;
BOOLEAN			autoscale_lx	= TRUE;
BOOLEAN			autoscale_ly	= TRUE;
BOOLEAN			autoscale_lz	= TRUE;
BOOLEAN 	  	clip_points	= FALSE;
BOOLEAN 	  	clip_lines1	= TRUE;
BOOLEAN 	  	clip_lines2	= FALSE;
BOOLEAN			draw_border	= TRUE;
BOOLEAN			draw_surface    = TRUE;
char			dummy_var[MAX_NUM_VAR][MAX_ID_LEN+1] = { "x", "y" };
char			xformat[MAX_ID_LEN+1] = DEF_FORMAT;
char			yformat[MAX_ID_LEN+1] = DEF_FORMAT;
char			zformat[MAX_ID_LEN+1] = DEF_FORMAT;
BOOLEAN			grid		= FALSE;
int				key			= -1;	/* default position */
float			key_x, key_y, key_z;	/* user specified position for key */
BOOLEAN			log_x		= FALSE,
			log_y		= FALSE,
			log_z		= FALSE;
FILE*			outfile		= stdout;
char			outstr[MAX_ID_LEN+1] = "STDOUT";
BOOLEAN			polar		= FALSE;
BOOLEAN			parametric	= TRUE;
int			samples		= SAMPLES;
int			iso_samples	= ISO_SAMPLES;
float			xsize		= 1.0;  /* scale factor for size */
float			ysize		= 1.0;  /* scale factor for size */
float			zsize		= 1.0;  /* scale factor for size */
float			surface_rot_z   = 30.0; /* Default 3d transform. */
float			surface_rot_x   = 60.0;
float			surface_scale   = 1.0;
float			surface_zscale  = 1.0;
int			term		= 0;		/* unknown term is 0 */
char			title[MAX_LINE_LEN+1] = "";
char			xlabel[MAX_LINE_LEN+1] = "";
char			ylabel[MAX_LINE_LEN+1] = "";
char			zlabel[MAX_LINE_LEN+1] = "";
int			title_xoffset	= 0;
int			title_yoffset	= 0;
int			xlabel_xoffset	= 0;
int			xlabel_yoffset	= 0;
int			ylabel_xoffset	= 0;
int			ylabel_yoffset	= 0;
int			zlabel_xoffset	= 0;
int			zlabel_yoffset	= 0;
float			tmin		= -5.0,
			tmax		=  5.0,
			umin		= -5.0,
			umax		= 5.0,
			vmin		= -5.0,
			vmax		= 5.0,
			xmin		= -10.0,
			xmax		= 10.0,
			ymin		= -10.0,
			ymax		= 10.0,
			zmin		= -10.0,
			zmax		= 10.0;
float			loff		= 0.0,
			roff		= 0.0,
			toff		= 0.0,
			boff		= 0.0;
int			draw_contour	= CONTOUR_NONE;
int			contour_pts	= 5;
int			contour_kind	= CONTOUR_KIND_LINEAR;
int			contour_order	= 4;
int			contour_levels	= 5;
float			zero = ZERO;			/* zero threshold, not 0! */

BOOLEAN xzeroaxis = TRUE;
BOOLEAN yzeroaxis = TRUE;

BOOLEAN xtics = TRUE;
BOOLEAN ytics = TRUE;
BOOLEAN ztics = TRUE;

float ticslevel = 0.5;

struct ticdef xticdef = {TIC_COMPUTED};
struct ticdef yticdef = {TIC_COMPUTED};
struct ticdef zticdef = {TIC_COMPUTED};

BOOLEAN			tic_in		= TRUE;

struct text_label *first_label = NULL;
struct arrow_def *first_arrow = NULL;


/* Color flags */

int color_mode = 1;


/* load TIC_USER definition */
/* (tic[,tic]...)
 * where tic is ["string"] value
 * Left paren is already scanned off before entry.
 */

void load_tic_user(struct ticdef* tdef)
{
    struct ticmark *list = NULL; /* start of list */
    struct ticmark *last = NULL; /* end of list */
    struct ticmark *tic = NULL; /* new ticmark */
    int i;

    for(i=0; i<iso_samples; i++) {
	   /* parse a new ticmark */
	   tic = (struct ticmark *)terrain_alloc(sizeof(struct ticmark));
	   if (tic == (struct ticmark *)NULL) {
		  free_marklist(list);
		  terrain_error("out of memory for tic mark");
	   }

	   tic->position = i;
	   tic->label = terrain_alloc(100);


	   printf("Label for curve %d: ", i);
	   scanf("%s", tic->label);

	   tic->next = NULL;

	   /* append to list */
	   if (list == NULL)
		last = list = tic;	/* new list */
	   else {				/* append to list */
		  last->next = tic;
		  last = tic;
	   }

    }
    
    /* successful list */
    if (tdef->type == TIC_USER) {
	   free_marklist(tdef->def.user);
	   tdef->def.user = NULL;
    }
    tdef->type = TIC_USER;
    tdef->def.user = list;
}

void free_marklist(struct ticmark* list)
{
    register struct ticmark *freeable;

    while (list != NULL) {
	   freeable = list;
	   list = list->next;
	   if (freeable->label != NULL)
		free( (char *)freeable->label );
	   free( (char *)freeable );
    }
}

void
changeDecimal(int decimal)
{
   switch(decimal)
   {
       case 0: strcpy(zformat, "%g");
	       break;

       case 1: strcpy(zformat, "%.1f");
	       break;

       case 2: strcpy(zformat, "%.2f");
	       break;
     
       case 3: strcpy(zformat, "%.3f");
	       break;

       case 4: strcpy(zformat, "%.4f");
	       break;


    }

}

