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
 * Adapted by Chi-Ting Lam for Terrain Plot. (5/10/1992)
 * Modified to handle multiple curves.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1989, 1990 Barton P. Miller,\
 Morgan Clark, Timothy Torzewski, Jeff Hollingsworth, and Bruce Irvin.\
 All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/visiClients/terrain/src/smooth.c,v 1.2 1997/05/21 03:20:34 tung Exp $";
#endif

/*
 * $Log: smooth.c,v $
 * Revision 1.2  1997/05/21 03:20:34  tung
 * Revised.
 *
 * Revision 1.1  1997/05/12 20:15:41  naim
 * Adding "Terrain" visualization to paradyn (commited by naim, done by tung).
 *
 * Revision 1.1  1992/05/19  07:19:28  lam
 * Initial revision
 *
 *
 * Revision 1.7  1992/04/25  14:39:44  lam
 * Changed doubles to floats.
 *
 * Revision 1.6  1992/04/20  20:32:15  lam
 * Malloc for points instead of the whole plot.
 * misc bug fixes.
 *
 * Revision 1.5  1992/03/29  16:49:17  lam
 * Added checks for extremely small graphs.
 *
 * Revision 1.3  1992/03/24  20:55:12  lam
 * Changed according to the simplified surface_points structure.
 *
 * Revision 1.2  1992/03/23  23:30:24  lam
 * Fixed a silly bug which caused MIPS complaining about memory
 * aliment.
 *
 * Revision 1.1  1992/03/21  03:49:02  lam
 * Initial revision (adapted from IPS)
 *
 * Revision 2.5  1991/03/14  20:48:17  hollings
 * Fixed $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/visiClients/terrain/src/smooth.c,v 1.2 1997/05/21 03:20:34 tung Exp $ definition.
 *
 * Revision 2.4  1990/09/05  16:38:56  rbi
 * Sequent merge.
 *
 * Revision 2.3  90/02/06  18:16:24  hollings
 * New Copyright
 * 
 * Revision 2.2  90/02/06  18:04:09  hollings
 * New Copyright
 * 
 * Revision 2.1  89/11/06  16:43:13  hollings
 * New Copyright Message
 * 
 * Revision 2.0  89/10/18  17:47:12  hollings
 * Toolkit Release
 * 
 * Revision 1.3  89/08/23  14:06:55  hollings
 * Big Bug Fix
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include "plot.h"
#include "smooth.h"
#include "setshow.h"
#include "misc.h"


#define		MAX_WIN_SIZE  	32 	/* max number of elements in window */
static int win_coeffs[MAX_WIN_SIZE];	/* indicate weighting for each slot */
static int coeff_sum;			/* sum of all window coefficients */
static int win_size;


static void shift_win (float window[], float newelem)
{
    int i;
    
    for (i=0; i < win_size-1; i++)
        window[i] = window[i+1];  
    window[win_size-1] = newelem;
}





void smooth (struct surface_points *new_surface, struct surface_points *surface,
	     int wsize)
{
    float avg_elem();
    float window[MAX_WIN_SIZE];	
    int win_bdry;         /* number of times to replicate endpoints */ 
			  /* win_bdry must be wsize/2 and wsize must be odd */
    int i, row, row_pt;

    /* set weights on sliding window */
    if (wsize > MAX_WIN_SIZE) 
       wsize = MAX_WIN_SIZE;

    /* Beware of exteremely small graphs */
    if (wsize > surface->samples)
       wsize = surface->samples;

    win_size = wsize;
    win_bdry = wsize / 2;
    coeff_sum = 0;
    for (i=0; i < win_bdry; i++) {
	win_coeffs[i] = win_coeffs[wsize - 1 - i] = i + 1;
	coeff_sum = coeff_sum + 2*(i+1);
    }
    if ((wsize % 2) != 0) {
	win_coeffs[win_bdry] = win_bdry + 1;
	coeff_sum = coeff_sum + win_bdry + 1;
    } 

    /*********** not going to use in dynamic graph *********************/
    /* Get rid of the old surface */
    /* free(new_surface->points); */

    /* Most parameters of smoothed graph are the same as the normal one */
    memcpy(new_surface, surface, sizeof(struct surface_points));
    /* allocate memory for the new surface */
    new_surface->points = (struct coordinate *)
	terrain_alloc(sizeof(struct coordinate) * new_surface->p_count);

    for (row = 0; row < new_surface->iso_samples; row++) {
      row_pt = row * surface->samples;

      /* boundary conditions: replicate first element WIN_BDRY times */
      for (i=0; i < win_bdry; i++)
	window[i] = surface->points[row_pt + 0].z;
      for (i=win_bdry; i < wsize; i++)
	window[i] = surface->points[row_pt + i - win_bdry].z;

      /* scan and smooth entire column */
      /* first we do up until the last few elements */
      for (i=0; i < surface->samples - (1 + win_bdry); i++) {
	new_surface->points[row_pt + i].x = surface->points[row_pt + i].x;
	new_surface->points[row_pt + i].y = surface->points[row_pt + i].y;
	new_surface->points[row_pt + i].z = avg_elem (window);
	shift_win (window, surface->points[row_pt + i + win_bdry + 1].z);
      }
      /* now finish the histogram: now oldhist[numBins-1] has just entered the window */
      for (i = surface->samples - (1 + win_bdry); i < surface->samples; i++) {
	new_surface->points[row_pt + i].x = surface->points[row_pt + i].x;
	new_surface->points[row_pt + i].y = surface->points[row_pt + i].y;
	new_surface->points[row_pt + i].z = avg_elem (window);
	shift_win (window, surface->points[row_pt + surface->samples-1].z);
      }
    }
}


static float avg_elem (float window[])
{
    int i;
    float avg = 0.0;

    for (i=0; i<win_size; i++)
	avg += window[i] * win_coeffs[i];
    avg /= coeff_sum;
    return (avg);
}


void smooth_med (struct surface_points *new_surface, struct surface_points *surface,
		 int wsize)
{
    float med_elem();
    float window[MAX_WIN_SIZE];	
    int win_bdry;         /* number of times to replicate endpoints */ 
			  /* win_bdry must be wsize/2 and wsize must be odd */
    int i, row, row_pt;

    /* set weights on sliding window */
    if (wsize > MAX_WIN_SIZE) 
       wsize = MAX_WIN_SIZE;

    /* Beware of exteremely small graphs */
    if (wsize > surface->samples)
       wsize = surface->samples;

    win_size = wsize;
    win_bdry = wsize / 2;
    coeff_sum = 0;
    for (i=0; i < win_bdry; i++) {
	win_coeffs[i] = win_coeffs[wsize - 1 - i] = i + 1;
	coeff_sum = coeff_sum + 2*(i+1);
    }
    if ((wsize % 2) != 0) {
	win_coeffs[win_bdry] = win_bdry + 1;
	coeff_sum = coeff_sum + win_bdry + 1;
    } 

    /**************** not going to use in the dynamic graph *******************/
    /* Get rid of the old surface */ 
    /* free(new_surface->points); */

    /* Most parameters of smoothed graph are the same as the normal one */
    memcpy(new_surface, surface, sizeof(struct surface_points));

    /* allocate memory for the new surface */
    new_surface->points = (struct coordinate *)
	terrain_alloc(sizeof(struct coordinate) * new_surface->p_count);

    for (row = 0; row < new_surface->iso_samples; row++) {
      row_pt = row * surface->samples;

      /* boundary conditions: replicate first element WIN_BDRY times */
      for (i=0; i < win_bdry; i++)
	window[i] = surface->points[row_pt + 0].z;
      for (i=win_bdry; i < wsize; i++)
	window[i] = surface->points[row_pt + i - win_bdry].z;

      /* scan and smooth entire column */
      /* first we do up until the last few elements */
      for (i=0; i < surface->samples - (1 + win_bdry); i++) {
	new_surface->points[row_pt + i].x = surface->points[row_pt + i].x;
	new_surface->points[row_pt + i].y = surface->points[row_pt + i].y;
	new_surface->points[row_pt + i].z = med_elem (window);
	shift_win (window, surface->points[row_pt + i + win_bdry + 1].z);
      }
      /* now finish the histogram: now oldhist[numBins-1] has just entered the window */
      for (i = surface->samples - (1 + win_bdry); i < surface->samples; i++) {
	new_surface->points[row_pt + i].x = surface->points[row_pt + i].x;
	new_surface->points[row_pt + i].y = surface->points[row_pt + i].y;
	new_surface->points[row_pt + i].z = med_elem (window);
	shift_win (window, surface->points[row_pt + surface->samples-1].z);
      }
    }
}


static float sort_array[MAX_WIN_SIZE];

static float med_elem (float window[])
{

    int i,j;
    float tmp;

    for (i=0; i<win_size; i++)
	sort_array[i] = window[i];

    /* Bubblee sort.  Bad, but good enough as a prototype */

    for (i = win_size - 1; i>=0; i--)
	for (j=0; j<i; j++)
	    if (sort_array[j] < sort_array[j+1]) {
		tmp = sort_array[j];
		sort_array[j] = sort_array[j+1];
		sort_array[j+1] = tmp;
	    }


    if (win_size % 2)
	return sort_array[win_size/2];
    else
	return (sort_array[win_size/2] + sort_array[(win_size + 1)/2])/2;
    
}









