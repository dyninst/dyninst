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
 * misc.c - misc utility routines.
 *
 * $Id: misc.c,v 1.7 2001/06/12 19:56:12 schendel Exp $
 */

#ifdef i386_unknown_linux2_0
#define _HAVE_STRING_ARCH_strcpy  /* gets rid of warnings */
#define _HAVE_STRING_ARCH_strsep
#endif

#include <stdio.h>
#include <stdlib.h>

#include "terrain.h"
#include "misc.h"
#include "plot.h"
#include "command.h"
#include "error.h"

char *ta_new_mem;		/* allocated memory */


char* terrain_mem_error(char* msgs)
{
   fprintf(stderr, "%s\n", msgs);
   quit3d();
   return NULL;
}


void
terrain_error(char* msgs)
{
   char *dupMsgs;		/* Copy of the message */
   /* popUpMsgs is distructive to the message string */
   
   dupMsgs = (char *) terrain_alloc(strlen(msgs) + 1);
   strcpy(dupMsgs, msgs);
 
   popUpMsgs((int)"Terrain Error", dupMsgs);
   quit3d();
   
}


void
terrain_warning(char* msgs)
{
   char *dupMsgs;		/* Copy of the message */
   /* popUpMsgs is distructive to the message string */
   
   dupMsgs = (char *) terrain_alloc(strlen(msgs) + 1);
   strcpy(dupMsgs, msgs);
 
   popUpMsgs((int)"Terrain Warning", dupMsgs);

   free(dupMsgs);
}


void free_surface(struct surface_points* surface)
{
   free(surface->points);
   free(surface);
}
    

int quit3d()
{
  kill_surface();
   
  XClearArea(dpy, win, 0, 0, 0, 0, True);

   exit(0);
}


