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
 * miscx.c - Assorted X routines.
 *
 * $Id: miscx.c,v 1.5 2002/02/11 22:08:08 tlmiller Exp $
 */

#ifdef i386_unknown_linux2_0 || defined(ia64_unknown_linux2_4)
#define _HAVE_STRING_ARCH_strcpy  /* gets rid of warnings */
#define _HAVE_STRING_ARCH_strsep
#endif

/* This must be first */
#include "terrain.h"
#include "Xbase.h"
#include "action.h"
#include "miscx.h"

#define FONTASCENT ((fontStruct->max_bounds).ascent)

XFontStruct 	*fontStruct;
int 		fontHeight, fontWidth;


void PopUpInit()
{
   fontStruct = rv.font;
   fontHeight = vchar;
   fontWidth = XTextWidth(fontStruct, "M", 1);
   InitAction(100,100);
}


void IFeep()
{
    XBell(dpy, 0);
}


int IGetGeometry(Window win, WindowInfo *Info)
{
    return(XGetGeometry(dpy, win, &(Info->root), &(Info->x), &(Info->y), &(Info->width), &(Info->height), &(Info->bdrwidth), &(Info->depth)));
}

/* The X tool kit should have had this function defined  */
void XtGetValue(Widget widget, char *field, caddr_t result)                       
{
    Arg AList[1];
    
    XtSetArg(AList[0], field, result);
    XtGetValues(widget, AList, 1);
}

/* The X tool kit should have had this function defined  */
void XtSetValue(Widget widget, char *field, caddr_t result)                       
{
    Arg AList[1];
    
    XtSetArg(AList[0], field, result);
    XtSetValues(widget, AList, 1);
}

















