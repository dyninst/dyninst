#ifndef _XawFormatBox_h
#define _XawFormatBox_h

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
 * Public interface to formatted box widget 
 *
 * $Id: FormatBox.h,v 1.5 2001/06/12 19:56:11 schendel Exp $
 */


/* give users of the widget the normal box stuff too */
#include <X11/Xaw/Box.h>

#define XtNleft "left"
#define XtNcenter "center"
#define XtNpropSpace "propSpace"

#define	XtNspanning "spanning"

#define XtNformat "format"

#define XtCFormat "Format"
#define XtCSpanning "spanning"

#define XtRFormatOption	"formatOption"

typedef struct _FormatBoxWidgetClass *FormatBoxWidgetClass;
typedef struct _FormatBoxRec *FormatBoxWidget;

extern WidgetClass formatBoxWidgetClass;

void StringToOption(XrmValue *args, int a_count, XrmValue *from, XrmValue *to);
void InitStuff(void);
void ChangedManaged(FormatBoxWidget w);
void Resize(FormatBoxWidget w);


#endif
