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

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1989, 1990 Barton P. Miller,\
 Morgan Clark, Timothy Torzewski, Jeff Hollingsworth, and Bruce Irvin.\
 All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/visiClients/terrain/src/Attic/shadow.c,v 1.1 1997/05/12 20:15:40 naim Exp $";
#endif

/*
 * shadow.c - produce a shadow (outline) of a plot.
 *
 * $Log: shadow.c,v $
 * Revision 1.1  1997/05/12 20:15:40  naim
 * Adding "Terrain" visualization to paradyn (commited by naim, done by tung).
 *
 * Revision 1.1  1992/05/19  06:30:55  lam
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include "plot.h"
#include "misc.h"
#include "setshow.h"

#define max(x,y) (((x) > (y))? (x) : (y))


/*********************************************************************
* shadow -- Produce a shadow plot from a given plot which is has
*            only few points but preserves the overall shape of the
*            old plot.  shadow() is very similar to reduce() except
*            that it use maximun instead of average.
*
* Parameters:
*   new_surface:  The shadow plot.
*   surface:      The plot to be reduced (not changed).
*   new_size:     Size of the reduced plot.  This is only a "suggestion".
*                 The actually size of the new plot will be somewhere 
*                 between new_size and new_size * 2.
*
* Bugs:
*   Assumed new_surface is defined and there are points associated to it.
*   new_surface and surface cannot be the same object.
*
*********************************************************************/

shadow(new_surface, surface, new_size)
struct surface_points *new_surface;
struct surface_points *surface;
int new_size;
{
  int cur_row, cur_col, org_col;/* Current row and column */
  int old_pt, new_pt;		/* Current point in old plot and new plot */
  float cur_max;		/* Variables for calculating the maxs */ 
  int skip;                     /* Skip factor */
  int odd;                      /* If skip is odd or not */
  int samples;                  /* Number of samples in the new surface */
  /* int iso_samples is in setshow.c */

  /* Get rid of old points in any case */
  free(new_surface->points);

  /* Check if the reduction is needed */
  if (surface->samples <= new_size * 2) {

    /* No reduction is needed.  Just plain copy. */
    memcpy(new_surface, surface, sizeof(struct surface_points));

    samples = sizeof(struct coordinate) * surface->p_count;

    new_surface->points = (struct coordinate *)
      terrain_alloc(sizeof(struct coordinate) * new_surface->p_count);

    memcpy(new_surface->points, surface->points,
	   sizeof(struct coordinate) * new_surface->p_count);

    return 0;
  }

  /* Even a reduced graph has at least two points in a row */
  new_size = (new_size > 2)? new_size : 2;
  
  skip = surface->samples / new_size;
  odd = skip % 2;
  
  samples = surface->samples / skip;
  
  /* One more point at the end if there are some left over points */
  samples += ((surface->samples % skip) > 0);
  
  /* Initialize the parameters of the reduced plot */

  new_surface->next_sp = NULL;
  new_surface->line_type = surface->line_type;
  new_surface->iso_samples = surface->iso_samples;
  new_surface->samples = samples;
  new_surface->p_count = surface->iso_samples * samples;
  new_surface->points = (struct coordinate *)
      terrain_alloc(sizeof(struct coordinate) * new_surface->p_count);
  new_surface->y_max = surface->y_max;
  new_surface->y_min = surface->y_min;
  new_surface->z_max = surface->z_max; 
  new_surface->z_min = surface->z_min;
  
  for (cur_row = 0; cur_row < new_surface->iso_samples; cur_row++) {
    
    /* Take care the first point */
    
    new_pt = cur_row * new_surface->samples;
    old_pt = cur_row * surface->samples;
    new_surface->points[new_pt].x = surface->points[old_pt].x;
    new_surface->points[new_pt].y = surface->points[old_pt].y;
    
    /* First point only weight half because it is on the boundry */
    cur_max = surface->points[old_pt].z;
    
    for (org_col = 0; org_col < (skip-1)/2; org_col++)
      cur_max = max(cur_max, surface->points[old_pt + org_col].z);
    
    /* If we skip even points, we have "half" points to go */
    if (!odd)
      cur_max = max(cur_max,  surface->points[old_pt + org_col].z);
    
    new_surface->points[new_pt].z = cur_max;
    
    /* Now take care all points in the middle */
    
    for (cur_col = 1; cur_col < new_surface->samples - 1; cur_col ++) {
      
      new_pt++;
      old_pt+=skip;
      
#ifdef REDUCE_DEBUG
      printf("new_pt = %d   old_pt = %d\n", new_pt, old_pt);
#endif
      
      new_surface->points[new_pt].x = (odd)?
	surface->points[old_pt].x
	  :
        max(surface->points[old_pt-1].x, surface->points[old_pt+1].x);
      
      new_surface->points[new_pt].y = surface->points[old_pt].y;
      
      /* skipping even points needs "half" points at the beginning */
      /* and at the end.                                           */
      
      cur_max = (odd)?
	0.0
	  :
        max(surface->points[old_pt-(skip/2)].z, 
	    surface->points[old_pt+(skip/2)].z);
      
      
      for (org_col= - (skip-1)/2; org_col <= (skip-1)/2; org_col++)
	cur_max = max(cur_max, surface->points[old_pt + org_col].z);
      
      new_surface->points[new_pt].z = cur_max;
    }
    
    new_pt++;

    /* Take care the last point for the current row.     */
    /* (i.e. -1 point of the next row)                   */

    old_pt = (cur_row + 1) * surface->samples - 1;
    
    new_surface->points[new_pt].x = surface->points[old_pt].x;
    new_surface->points[new_pt].y = surface->points[old_pt].y;
    
    cur_max = 0.0;
    for (org_col = -(skip/2); org_col<=0; org_col++)
      cur_max = max(cur_max, surface->points[old_pt + org_col].z);

    new_surface->points[new_pt].z = cur_max;
    
#ifdef REDUCE_DEBUG
    printf("-----\n");
#endif
  }
  
#ifdef REDUCE_DEBUG
  for (cur_row=0; cur_row < new_surface->iso_samples; cur_row++)
    for (cur_col=0; cur_col < new_surface->samples; cur_col++) {
      new_pt = cur_row * new_surface->samples + cur_col;
      printf("%3.2f %3.2f %3.2f\n", 
	     new_surface->points[new_pt].x,
	     new_surface->points[new_pt].y,
	     new_surface->points[new_pt].z);
    }
#endif
  
#ifdef REDUCE_DEBUG 
  printf("min z = %f   max z = %f\n", new_surface->z_min, new_surface->z_max);
#endif
  
  return 0;
}

















