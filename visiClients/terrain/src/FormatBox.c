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
 * FormatBox.c - Formated box widget
 * 
 * $Id: FormatBox.c,v 1.6 2001/06/12 19:56:11 schendel Exp $
 */

#ifdef i386_unknown_linux2_0
#define _HAVE_STRING_ARCH_strcpy  /* gets rid of warnings */
#define _HAVE_STRING_ARCH_strsep
#endif

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/Misc.h>
#include <X11/Xaw/XawInit.h>
#include <stdio.h>

#include "FormatBoxP.h"

/* Resources */
static XtResource resources[] = {
    { XtNformat, XtCFormat, XtRFormatOption, sizeof(int),
	XtOffsetOf(FormatBoxRec, format.justify_mode),
	XtRImmediate, (XtPointer) XtFormatNone },
    { XtNspanning, XtCSpanning, XtRBoolean, sizeof(Boolean),
	XtOffsetOf(FormatBoxRec, format.maximize),
	XtRImmediate, (XtPointer) False },
};

FormatBoxClassRec formatBoxClassRec = {
  {
/* core_class fields      */
    /* superclass         */    (WidgetClass) &boxClassRec,
    /* class_name         */    "FormatedBox",
    /* widget_size        */    sizeof(FormatBoxRec),
    /* class_initialize   */    InitStuff,
    /* class_part_init    */	NULL,
    /* class_inited       */	FALSE,
    /* initialize         */    NULL,
    /* initialize_hook    */	NULL,
    /* realize            */    XtInheritRealize,
    /* actions            */    NULL,
    /* num_actions	  */	0,
    /* resources          */    resources,
    /* num_resources      */    XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	TRUE,
    /* compress_enterleave*/	TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    Resize,
    /* expose             */    NULL,
    /* set_values         */    NULL,
    /* set_values_hook    */	NULL,
    /* set_values_almost  */    XtInheritSetValuesAlmost,
    /* get_values_hook    */	NULL,
    /* accept_focus       */    NULL,
    /* version            */	XtVersion,
    /* callback_private   */    NULL,
    /* tm_table           */    NULL,
    /* query_geometry     */	(XtGeometryHandler) XtInheritQueryGeometry,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension          */	NULL
  },{
/* composite_class fields */
    /* geometry_manager   */    (XtGeometryHandler) XtInheritGeometryManager,
    /* change_managed     */    ChangedManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* extension          */	NULL
  },{
/* Box class fields */
    /* empty		  */	0,
  },{
/* Format Box class fields */
    /* empty              */    0,
  }
};

WidgetClass formatBoxWidgetClass = (WidgetClass)&formatBoxClassRec;

/* resource converter for formatting */
void StringToOption(XrmValue *args, int a_count, XrmValue *from, XrmValue *to)
{
    static int mode;
    char string[80];

    to->size = sizeof(int);
    to->addr = (caddr_t) &mode;
    if (!strcmp(from->addr, XtNleft)) {
	mode = XtFormatNone;
    } else if (!strcmp(from->addr, XtNcenter)) {
	mode = XtFormatCenter;
    } else if (!strcmp(from->addr, XtNpropSpace)) {
	mode = XtFormatJustify;
    } else {
	sprintf(string, "Invalid formatting option %s", from->addr);
	XtWarning(string);
	mode = XtFormatNone;
    }

}

static void InitStuff()
{
    XtAddConverter(XtRString, XtRFormatOption, (XtConverter)StringToOption, NULL, 0);
}

/*
 * Guts of the format box widget.
 *
 */
static void Resize(FormatBoxWidget w)
{
    int h_space;
    char str[80];
    int x, extra;
    CoreWidget child;
    int i, width, height;

    if (w->format.maximize) {
	if (w->box.orientation == XtorientVertical) {
	    /* make width of children our width less spacing around edges */
	    width = w->core.width - 2 * w->box.h_space;
	    for (i = 0; (unsigned) i < w->composite.num_children; i++) {
		child = w->composite.children[i];
		XtResizeWidget(child, (unsigned) width - 2 * child->core.border_width, 
		    child->core.height, child->core.border_width);
	    }
	} else {
	    /* make height of children our height less spacing around edges */
	    height = w->core.height - 2 * w->box.v_space;
	    for (i = 0; (unsigned) i < w->composite.num_children; i++) {
		child = w->composite.children[i];
		XtResizeWidget(child, child->core.width, (unsigned) height, 
		    child->core.border_width);
	    }
	}
    }

    if (w->format.justify_mode == XtFormatNone) {
	return;
    } else {
	/* should add support for vertical justification */
	if (w->box.orientation == XtorientVertical) {
	    return;
	}

	/* compute used width */
	h_space = w->box.h_space;
	/* double count first h space */
	width = -h_space;
	for (i = 0; (unsigned) i < w->composite.num_children; i++) {
	    child = w->composite.children[i];
	    width += child->core.width + child->core.border_width * 2 + h_space;
	}

	extra = w->core.width - width;
	if (extra < 0) {
	    sprintf(str, "%s: Formated Box widget too small (size %d)",
		w->core.name, extra);
	    XtWarning(str);
	    return;
	}

	if (w->format.justify_mode == XtFormatCenter) {
	    extra /= 2;
	    x = extra;
	} else {
	    extra /= w->composite.num_children + 1;
	    x = extra;
	    h_space += extra;
	}

	/* now move the children */
	for (i = 0; (unsigned) i < w->composite.num_children; i++) {
	    child = w->composite.children[i];
	    XtMoveWidget(child, x, child->core.y);
	    x += child->core.width + child->core.border_width * 2 + h_space;
	}
    }
} 

/*
 * Call our superclass ChangedManaged function, and then force a call to resize
 *   to get the layout done to format the box.
 */
static void ChangedManaged(FormatBoxWidget w)
{
   CompositeClassRec *super= (CompositeClassRec *) 
       formatBoxClassRec.core_class.superclass;

   super->composite_class.change_managed((Widget)w);
   Resize(w);
}
