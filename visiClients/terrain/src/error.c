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

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/visiClients/terrain/src/error.c,v 1.2 1997/05/18 22:50:11 tung Exp $";
#endif

/* Modified by Chi-Ting Lam for terrain plot */

/* 
 * error.c - This file contains the code to generate an error window when
 *    an error occurs.  
 *
 * Revision 2.9  1991/09/19  22:28:42  rbi
 * Early multi-application support.
 *
 * Revision 2.8  1991/05/15  19:37:05  hollings
 * Changes in preparation for error description windows.
 *
 * Revision 2.7  1991/03/15  19:35:08  hollings
 * Support mutli-line messages.
 *
 * Revision 2.6  1991/03/14  20:48:17  hollings
 * Fixed start of program comments
 *
 * Revision 2.5  1990/05/16  15:23:23  hollings
 * Toolkit version of Action window.
 *
 * Revision 2.4  90/03/05  11:52:18  hollings
 * improved status/error messages.
 * 
 * Revision 2.3  90/02/06  18:15:27  hollings
 * New Copyright
 * 
 * Revision 2.2  90/02/06  18:03:12  hollings
 * New Copyright
 * 
 * Revision 2.1  89/11/06  16:42:04  hollings
 * New Copyright Message
 * 
 * Revision 2.0  89/10/18  17:46:11  hollings
 * Toolkit Release
 * 
 * Revision 1.8  89/10/18  15:32:01  hollings
 * X Toolkit upgrade - Pass #2
 * 
 * Revision 1.7  89/09/07  13:55:08  hollings
 * SPARC port cleanup
 * 
 * Revision 1.6  89/08/23  14:08:41  hollings
 * Big Bug Fix
 * 
 * Revision 1.5  89/08/09  13:33:48  hollings
 * ASYC application abort
 * 
 * Revision 1.4  89/08/02  16:22:16  hollings
 * Fixed malloc warning.
 * 
 * Revision 1.3  89/07/26  13:28:40  hollings
 * Improved error handling
 * 
 * Revision 1.2  89/07/20  11:20:12  hollings
 * made abort() the default action for now.
 * 
 * Revision 1.1  89/07/11  10:47:56  hollings
 * Initial revision
 * 
 *
 */
#include "config.h"

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xmu/Xmu.h>

#include "Xbase.h"		/* For WindowInfo structure */
#include "terrain.h"

#define BUTTONC	3
char *buttons[BUTTONC] = {
    "Continue",
    "Quit",
    "Dump Core",
};

ErrorWrapup(pressed)
int pressed;
{
    if (pressed == 0)
	return(0);
    else if (pressed == 1)
	exit(0);
    else 	/* dump core */
	abort();
}

/*
 * host is an optional parameter.
 *
 */

popUpMsgs(title, message)
int title;
char *message;
{
    
    char *ch;
    int lCount;
    int pressed;
    char *log[10];
    char line[250];

    WindowInfo winInfo;

    log[0] = message;
    lCount = 1;
    for (ch=message; *ch != '\0'; ch++) {
       if (*ch == '\n') {
	  log[lCount++] = ch+1;
	  *ch = '\0';
       }
    }

    /* Put the message at the middle of the terrain window */

    IGetGeometry(win, &winInfo);
    InitAction(winInfo.x + (winInfo.width>>1),
	       winInfo.y + (winInfo.height>>1));
 
    RequestAction(title, NULL, NULL, lCount, log, BUTTONC, buttons, 
	XtJustifyCenter, ErrorWrapup);
}

