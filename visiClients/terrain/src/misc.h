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
 * misc.h - header file of misc.c
 *
 * $Log: misc.h,v $
 * Revision 1.3  1997/05/21 03:20:32  tung
 * Revised.
 *
 * Revision 1.2  1997/05/19 16:03:41  tung
 * Remove unused files.
 *
 * Revision 1.1  1997/05/12 20:15:33  naim
 * Adding "Terrain" visualization to paradyn (commited by naim, done by tung).
 *
 * Revision 1.1  1992/05/19  07:21:56  lam
 * Initial revision
 *
 *
 */

#ifndef TERRAIN_MISC_GUARD
#define TERRAIN_MISC_GUARD

void terrain_error(char*);
char *terrain_mem_error(char*);
void terrain_warning(char*);
extern void free_surface();
int quit3d();

extern char *ta_new_mem;

#define terrain_alloc(size) \
((void *)(((ta_new_mem  = (char *) malloc(size)) == NULL)?\
   terrain_mem_error("Memory allocation error!") : ta_new_mem))

#endif


