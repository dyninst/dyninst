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
 * $Id: misc.h,v 1.5 2001/06/12 19:56:12 schendel Exp $
 */

#ifndef TERRAIN_MISC_GUARD
#define TERRAIN_MISC_GUARD

#include "plot.h"

void terrain_error(char*);
char *terrain_mem_error(char*);
void terrain_warning(char*);
extern void free_surface(struct surface_points* surface);
int quit3d(void);

extern char *ta_new_mem;

#define terrain_alloc(size) \
((void *)(((ta_new_mem  = (char *) malloc(size)) == NULL)?\
   terrain_mem_error("Memory allocation error!") : ta_new_mem))

#endif


