/* 
 * Copyright (c) 1989, 1990 Barton P. Miller, Morgan Clark, Timothy Torzewski,
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
 * FormatBoxP.h - Private definitions for Formatted box widget
 * 
 * $Id: FormatBoxP.h,v 1.2 1998/03/30 01:22:16 wylie Exp $
 */

#ifndef _XawFormatBoxP_h
#define _XawFormatBoxP_h

#include "FormatBox.h"
#include <X11/Xaw/BoxP.h>
#include <X11/CompositeP.h>
#include <X11/Xmu/Converters.h>

/* New fields for the Box widget class record */
typedef struct {int empty;} FormatClassPart;

/* Full class record declaration */
typedef struct _FormatBoxClassRec {
    CoreClassPart	core_class;
    CompositeClassPart  composite_class;
    BoxClassPart	box_class;
    FormatClassPart	format_class;
} FormatBoxClassRec;

extern FormatBoxClassRec formatBoxClassRec;

/* New fields for the Box widget record */
typedef struct {
    int		justify_mode;
    int		maximize;
} FormatBoxPart;


typedef struct _FormatBoxRec {
    CorePart	    core;
    CompositePart   composite;
    BoxPart 	    box;
    FormatBoxPart   format;
} FormatBoxRec;

/* formatting options */
#define XtFormatNone	0	/* default box behavior */
#define XtFormatCenter	1	/* center children, but don't re-space */
#define XtFormatJustify	2	/* full proportional spacing */

#endif 
