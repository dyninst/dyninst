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

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/visiClients/terrain/src/Attic/ipsstuff.c,v 1.1 1997/05/12 20:15:29 naim Exp $";
#endif

/*
 * ipsstuff.c - communication between terrain and ips.
 *
 * $Log: ipsstuff.c,v $
 * Revision 1.1  1997/05/12 20:15:29  naim
 * Adding "Terrain" visualization to paradyn (commited by naim, done by tung).
 *
 * Revision 1.3  1992/05/19  18:43:40  lam
 * Correction on error messages.
 *
 * Revision 1.2  1992/05/19  17:32:29  lam
 * Allows user to reselect a node when the selected node is not
 * suitable for terrain plot instead of just die.
 * Uses pop-up windows for error/warning messages.
 *
 * Revision 1.1  1992/05/19  06:30:55  lam
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <evu_lib.h>
#include "plot.h"
#include "setshow.h"


Metric *ips_get_data(nmetrics)
int *nmetrics;
{
    NodeName *name;		/* The node selected */
    Metric *metric;		/* The metric of the node selectec */

    int done = 0;		/* Done selecting a node */

    /* Keep asking until we get a valid node for terrain */

    while (!done) {
       if ((name = EVU_GetSelectedNode()) == NULL)
	  terrain_warning("No node is selected.\nPlease select a node and select\n\"Continue\".");
       else {
	  if (++name -> level == IPS_NLEVELS) {
	     terrain_warning("Selected node has no children.\nPlease select another node\nand select\"Continue\".");
	     free(name);
	  } else {
	     done = 1;
	  }
       }
    }

    name ->components[name -> level] = NULL;

    if((metric = EVU_GetThisMetric(NULL, name, HISTO, nmetrics)) == NULL){
	terrain_error("Cannot get the metric of the selected node.");
	exit(1);
    }

    return metric;
}

#define SHOWPOINTS 20

void ips_get_3ddata(nmetrics, metric, this_plot)
int nmetrics;
Metric metric[];
struct surface_points *this_plot;
{
    int trimfactor;
    int i, j, numbins = metric[0].data -> hdata.numbins, npoints;
    int trimbins;
    float time_per_bin;

    if (numbins > SHOWPOINTS) 
      trimfactor = metric[0].data -> hdata.numbins / SHOWPOINTS;
    else 
      trimfactor = 1;

    trimbins = (numbins / trimfactor);
    time_per_bin = metric[0].data -> hdata.time_per_bin * trimfactor;
    npoints = nmetrics * trimbins;

    for(i = 0; i < nmetrics; ++i){
	float z,zz;

	for(j = 0; j < trimbins; j++) {
            float normalizedj = j * time_per_bin;
            int jj;
        
	    zz = 0.0;
	    for (jj = j*trimfactor; jj < j*trimfactor+trimfactor; jj++) {
	      zz += metric[i].data -> hdata.bins[jj];
	    }
	    z = zz / trimfactor;

	    this_plot -> points[i * trimbins + j].x = normalizedj;
	    this_plot -> points[i * trimbins + j].y = i;
	    /* z = metric[i].data -> hdata.bins[j];*/
	    this_plot -> points[i * trimbins + j].z = z;

	    if (normalizedj < xmin) xmin = normalizedj;
	    if (normalizedj > xmax) xmax = normalizedj;
	    
	    if (i < ymin) ymin = i;
	    if (i > ymax) ymax = i;
	    
	    if (z < zmin) zmin = z;
	    if (z > zmax) zmax = z;
	}
    }

    this_plot -> p_count = npoints;

    for (i = npoints, j = 0; i < npoints*2; i++, j+=samples) {
	if (j >= npoints) j -= npoints-1;
	this_plot->points[i].x = this_plot->points[j].x;
	this_plot->points[i].y = this_plot->points[j].y;
	this_plot->points[i].z = this_plot->points[j].z;
    }
}


void ips_cleanup()
{
    EVU_Close();
}














