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

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/visiClients/terrain/src/misc.c,v 1.3 1997/05/19 01:00:09 tung Exp $";
#endif

/*
 * misc.c - misc utility routines.
 *
 * $Log: misc.c,v $
 * Revision 1.3  1997/05/19 01:00:09  tung
 * Eliminate ips dependent library files.
 *
 * Revision 1.2  1997/05/14 19:14:57  naim
 * Minor changes for sunos version of terrain - naim
 *
 * Revision 1.1  1997/05/12 20:15:32  naim
 * Adding "Terrain" visualization to paradyn (commited by naim, done by tung).
 *
 * Revision 1.2  1992/05/19  17:34:20  lam
 * All error messages are displayed in pop up windows instead of just
 * being dumped to stdout.
 * Added terrain_warning() which won't force terrain to exit.
 *
 * Revision 1.1  1992/05/19  06:30:55  lam
 * Initial revision
 *
 *
 */

#include <stdio.h>
/* #include <strings.h> */

#include "misc.h"
#include "plot.h"
#include "terrain.h"
#include "command.h"

char *ta_new_mem;		/* allocated memory */


char*
terrain_mem_error(msgs)
char *msgs;
{
   fprintf(stderr, "%s\n", msgs);
   quit3d();
}


void
terrain_error(msgs)
char *msgs;
{
   char *dupMsgs;		/* Copy of the message */
   /* popUpMsgs is distructive to the message string */
   
   dupMsgs = (char *) terrain_alloc(strlen(msgs) + 1);
   strcpy(dupMsgs, msgs);
 
   popUpMsgs("Terrain Error", dupMsgs);
   quit3d();
}


void
terrain_warning(msgs)
char *msgs;
{
   char *dupMsgs;		/* Copy of the message */
   /* popUpMsgs is distructive to the message string */
   
   dupMsgs = (char *) terrain_alloc(strlen(msgs) + 1);
   strcpy(dupMsgs, msgs);
 
   popUpMsgs("Terrain Warning", dupMsgs);

   free(dupMsgs);
}



free_surface(surface)
struct surface_points *surface;
{
   free(surface->points);
   free(surface);
}
    

quit3d()
{
  kill_surface();
   
  XClearArea(dpy, win, 0, 0, 0, 0, True);

   exit(0);
}


