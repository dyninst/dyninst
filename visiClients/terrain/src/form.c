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
 * form.c - define the basic frame of the window.
 *          scroll bars and menu routines.
 *
 * $Id: form.c,v 1.10 2000/10/17 17:29:00 schendel Exp $
 */

#include <stdio.h>

#include <X11/Intrinsic.h>	/* Include standard Toolkit Header file.
				   We do no need "StringDefs.h" */

#include <X11/Xaw/Command.h>
#include <X11/Xaw/Label.h>	/* Include the Label widget's header file. */
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Box.h> 
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/StringDefs.h>
#include "paradyn/xbm/logo.xbm"


#include <X11/StringDefs.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/Sme.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "menu.h"
#include "plot.h"
#include "setshow.h"
#include "command.h"
#include "form.h"
#include "terrain.h"
#include "misc.h"
#include "pdutilOld/h/pdsocket.h"
#include "visi/h/visualization.h"

#define FORM_H	h
#define FORM_W	w
#define TITLE_H	20
#define TITLE_W	FORM_W
#define INFO_H	20
#define INFO_W	(FORM_W-BAR_W)/2
#define BAR_W	20
#define HBAR_L	W-BAR_W
#define VBAR_L	H-TITLE_H

#define INFOH_FORMAT	"From x-z: %3.3f degs"
#define INFOV_FORMAT	"From top: %3.3f degs"


/* The informtion passed back to the callback handler */
typedef struct {
    Widget label;
    int value;
} rotInfo_t;


#define SMOOTH_POS 0		/* The location of smooth option in the menu */
#define MED_POS    1		/* The location of smoothing method in menu */

/* Some messages in the main menu */

#define FILEMENU_TITLE "File Menu"
#define VIEWMENU_TITLE "View Menu"
#define SMOOTH_MSG "Show Smooth Terrain"
#define UNSMOOTH_MSG "Show Raw Terrain"
#define AVG_MSG "Use Mean When Smoothing"
#define MED_MSG "Use Median When Smoothing"

/* New messages in the main menu for communication with the visilib of Paradyn */
#define QUIT_MSG "Close 3D Histogram"
#define RESROT_MSG "Reset Rotational View"

/* The main menu */
struct menuItem fileMenu[] = {
   { QUIT_MSG, (void *) quit3d },
   { NULL, NULL },
};

struct menuItem viewSmoothMenu[] = {
   { UNSMOOTH_MSG, (void *) plot_unsmooth },
   { MED_MSG, (void *) plot_usemed },
   { RESROT_MSG, (void *) reset_rotate },
   { NULL, NULL},
};

struct menuItem viewRawMenu[] = {
   { SMOOTH_MSG, (void *) plot_smooth },
   { RESROT_MSG, (void *) reset_rotate },
   { NULL, NULL},
};




Widget filemenuTitle, viewmenuTitle, filemenu, viewmenu;	
static Widget scrollH, scrollV;
/* Shared for menu modification */


/********************************************************************
* 
* createForm - Initialize the frame, scroll bars, menus for terrain
*              plot.
*
********************************************************************/

Widget createForm(Widget toplevel, int h, int w)
{
    Widget form;
    Widget mainWin, infoH, infoV, titleBar, logo;
    Arg arg[30];
    Pixmap *logobitmap;
    /* int depth; */
    char line[160];
    Display *dpy;
    int i;
    char info_label[80];

    /*
     * Create a Widget to display the string.  The label is picked up
     * from the resource database.
     */

    dpy = XtDisplay(toplevel);

    logobitmap = (Pixmap *)XCreateBitmapFromData( dpy,
				RootWindowOfScreen(XtScreen(toplevel)),
			  	logo_bits,
			 	logo_width,
				logo_height);


    i=0;
    XtSetArg(arg[i], XtNx, 0); i++;
    XtSetArg(arg[i], XtNy, 0); i++;
    XtSetArg(arg[i], XtNheight, FORM_H); i++;
    XtSetArg(arg[i], XtNwidth, FORM_W); i++;
    XtSetArg(arg[i], XtNborderWidth, 0); i++;
    XtSetArg(arg[i], XtNdefaultDistance, 0); i++;
    form = XtCreateManagedWidget("form", formWidgetClass, toplevel, arg, (Cardinal)i);

    /* Paradyn Logo */
    i = 0;
    XtSetArg(arg[i], XtNbitmap, logobitmap); i++;
/*  XtSetArg(arg[i], XtNfromHoriz, form); i++; */
    XtSetArg(arg[i], XtNshowGrip, False);  i++;
    XtSetArg(arg[i], XtNjustify, XtJustifyRight);  i++;
    XtSetArg(arg[i], XtNwidth, logo_width + 5);  i++;
    XtSetArg(arg[i], XtNheight, logo_height);  i++;
    XtSetArg(arg[i], XtNvertDistance, 0);  i++;
    XtSetArg(arg[i], XtNhorizDistance, (FORM_W - logo_width - 5));  i++;
    XtSetArg(arg[i], XtNright, XtChainRight); i++;
    XtSetArg(arg[i], XtNleft, XtChainRight); i++;
    XtSetArg(arg[i], XtNtop, XtChainTop); i++;
    XtSetArg(arg[i], XtNbottom, XtChainTop); i++;
    XtSetArg(arg[i], XtNresizable, False); i++;
    logo = XtCreateManagedWidget( "logo", labelWidgetClass, form, arg, (unsigned)i); 

    /* Menu bar */
    i = 0;
    XtSetArg(arg[i], XtNshowGrip, False);            i++;
    XtSetArg(arg[i], XtNlabel, "3D-Histogram Display"); i++;
    XtSetArg(arg[i], XtNleft, XtChainLeft); i++;  /* R4 have no XawChainTop */ 
    XtSetArg(arg[i], XtNright, XtChainRight); i++;
    XtSetArg(arg[i], XtNtop, XtChainTop); i++;
    XtSetArg(arg[i], XtNbottom, XtChainTop); i++;
    XtSetArg(arg[i], XtNvertDistance, 0);  i++;
    XtSetArg(arg[i], XtNborderWidth, 1);  i++;
    XtSetArg(arg[i], XtNhorizDistance, 0);  i++;
    XtSetArg(arg[i], XtNwidth, FORM_W - logo_width - 5);  i++;
    XtSetArg(arg[i], XtNheight, logo_height / 2);  i++;
    XtSetArg(arg[i], XtNborderWidth, 1);  i++;
    titleBar = XtCreateManagedWidget("titlebar", labelWidgetClass, form, arg, (unsigned)i);

    /* File Menu*/
    i=0;
    XtSetArg(arg[i], XtNvertDistance, logo_height / 2); i++;
    XtSetArg(arg[i], XtNheight, logo_height / 2); i++;
    XtSetArg(arg[i], XtNwidth, TITLE_H * 3); i++;
    XtSetArg(arg[i], XtNresizable, True);  i++;
    XtSetArg(arg[i], XtNleft, XtChainLeft); i++; /* R4 have no XawChainTop */
    XtSetArg(arg[i], XtNright, XtChainLeft); i++;
    XtSetArg(arg[i], XtNtop, XtChainTop); i++;
    XtSetArg(arg[i], XtNbottom, XtChainTop); i++;
    XtSetArg(arg[i], XtNborderWidth, 0);  i++;
    XtSetArg(arg[i], XtNlabel, "File"); i++;
    filemenuTitle = XtCreateManagedWidget("title", menuButtonWidgetClass, form, 
				  arg , (Cardinal)i);

    XtOverrideTranslations(filemenuTitle, XtParseTranslationTable("<Btn1Down>:XawPositionSimpleMenu(menu) MenuPopup(menu)"));

    filemenu = CreateMenu(filemenuTitle, FILEMENU_TITLE, fileMenu, NULL);

    /* Smooth Menu */
    i=0;
    XtSetArg(arg[i], XtNvertDistance, logo_height / 2); i++;
    XtSetArg(arg[i], XtNhorizDistance, TITLE_H * 4);  i++;    
    XtSetArg(arg[i], XtNheight, logo_height / 2); i++;
    XtSetArg(arg[i], XtNwidth, TITLE_H * 3); i++;
    XtSetArg(arg[i], XtNresizable, True);  i++;
    XtSetArg(arg[i], XtNleft, XtChainLeft); i++; /* R4 have no XawChainTop */
    XtSetArg(arg[i], XtNright, XtChainLeft); i++;
    XtSetArg(arg[i], XtNtop, XtChainTop); i++;
    XtSetArg(arg[i], XtNbottom, XtChainTop); i++;
    XtSetArg(arg[i], XtNborderWidth, 0);  i++;
    XtSetArg(arg[i], XtNlabel, "View"); i++;
    viewmenuTitle = XtCreateManagedWidget("title", menuButtonWidgetClass, form, 
				  arg , (Cardinal)i);

    XtOverrideTranslations(viewmenuTitle, XtParseTranslationTable("<Btn1Down>:XawPositionSimpleMenu(menu) MenuPopup(menu)"));

    viewmenu = CreateMenu(viewmenuTitle, VIEWMENU_TITLE, viewRawMenu, NULL);


    /* V-info box */

    i=0;
    XtSetArg(arg[i], XtNvertDistance, logo_height); i++;
    XtSetArg(arg[i], XtNhorizDistance, 0); i++;

    XtSetArg(arg[i], XtNheight, INFO_H); i++;
    XtSetArg(arg[i], XtNwidth, INFO_W); i++;

    XtSetArg(arg[i], XtNleft, XtChainLeft); i++;
    XtSetArg(arg[i], XtNright, XtRubber); i++;
    XtSetArg(arg[i], XtNtop, XtChainTop); i++;
    XtSetArg(arg[i], XtNbottom, XtChainTop); i++;

    sprintf(info_label, INFOV_FORMAT, surface_rot_x);
    XtSetArg(arg[i], XtNlabel, info_label); i++;

    infoV = XtCreateManagedWidget("infoV", labelWidgetClass, form, 
				  arg , (Cardinal)i);

    /* H-info box */

    i=0;
    XtSetArg(arg[i], XtNvertDistance, logo_height); i++;
    XtSetArg(arg[i], XtNhorizDistance, INFO_W); i++;

    XtSetArg(arg[i], XtNheight, INFO_H); i++;
    XtSetArg(arg[i], XtNwidth, INFO_W); i++;

    XtSetArg(arg[i], XtNleft, XtRubber); i++;
    XtSetArg(arg[i], XtNright, XtChainRight); i++;
    XtSetArg(arg[i], XtNtop, XtChainTop); i++;
    XtSetArg(arg[i], XtNbottom, XtChainTop); i++;

    sprintf(info_label, INFOH_FORMAT, surface_rot_z);
    XtSetArg(arg[i], XtNlabel, info_label); i++;

    infoH = XtCreateManagedWidget("infoH", labelWidgetClass, form, 
				  arg , (Cardinal)i);

    /* Main window */

    /* XtSetArg(arg[0], XtNdepth, 0);
    XtGetValues(toplevel, arg, (Cardinal)1); 
    depth = (int)(arg[0].value); */

    i=0;
    XtSetArg(arg[i], XtNvertDistance, TITLE_H * 2 + INFO_H); i++;
    XtSetArg(arg[i], XtNhorizDistance, 0); i++;

    XtSetArg(arg[i], XtNheight, FORM_H - TITLE_H - INFO_H - BAR_W); i++;
    XtSetArg(arg[i], XtNwidth, FORM_W  - BAR_W); i++;

    XtSetArg(arg[i], XtNleft, XtChainLeft); i++;
    XtSetArg(arg[i], XtNright, XtChainRight); i++;
    XtSetArg(arg[i], XtNtop, XtChainTop); i++;
    XtSetArg(arg[i], XtNbottom, XtChainBottom); i++;

    /* sprintf(line, "Depth = %d", depth); */
    XtSetArg(arg[i], XtNlabel, line); i++;

    mainWin = XtCreateManagedWidget("label1", labelWidgetClass, form,
				 arg, (Cardinal)i);


    /* Scroll bar at the bottom */

    i=0;
    XtSetArg(arg[i], XtNvertDistance, FORM_H + TITLE_H -BAR_W); i++;
    XtSetArg(arg[i], XtNhorizDistance, 0); i++;

    XtSetArg(arg[i], XtNheight, BAR_W); i++;
    XtSetArg(arg[i], XtNwidth, FORM_W-BAR_W); i++;

    XtSetArg(arg[i], XtNtop, XtChainBottom); i++;
    XtSetArg(arg[i], XtNbottom, XtChainBottom); i++;
    XtSetArg(arg[i], XtNleft, XtChainLeft); i++;
    XtSetArg(arg[i], XtNright, XtChainRight); i++;
 
    XtSetArg(arg[i], XtNshown, 1.0); i++;
    XtSetArg(arg[i], XtNorientation, XtorientHorizontal); i++;
    XtSetArg(arg[i], XtNminimumThumb, (Dimension) 30); i++;

    scrollH = XtCreateManagedWidget("scrollH", scrollbarWidgetClass, form,
				 arg, (Cardinal)i);

    XtOverrideTranslations(scrollH, XtParseTranslationTable("<Btn2Up>: NotifyScroll(Proportional) NotifyEndThumb() EndScroll()"));

    XawScrollbarSetThumb(scrollH, surface_rot_z / 360.0, -1.0);
    XtAddCallback(scrollH, XtNjumpProc, gotJumpH, (XtPointer) infoH);
    XtAddCallback(scrollH, XtNscrollProc, gotScrollH, (XtPointer) infoH);
    XtAddCallback(scrollH, XtNscrollProc, gotScrollH, (XtPointer) infoH);

    /* Scroll bar on right */

    i=0;
    XtSetArg(arg[i], XtNvertDistance, TITLE_H * 2 + 1); i++;
    XtSetArg(arg[i], XtNhorizDistance, FORM_W - BAR_W); i++;

    XtSetArg(arg[i], XtNheight, FORM_H - TITLE_H); i++;
    XtSetArg(arg[i], XtNwidth, BAR_W); i++;

    XtSetArg(arg[i], XtNleft, XtChainRight); i++;
    XtSetArg(arg[i], XtNright, XtChainRight); i++;
    XtSetArg(arg[i], XtNtop, XtChainTop); i++;
    XtSetArg(arg[i], XtNbottom, XtChainBottom); i++;

    XtSetArg(arg[i], XtNshown, 1.0); i++;
    XtSetArg(arg[i], XtNorientation, XtorientVertical); i++;
    XtSetArg(arg[i], XtNminimumThumb, (Dimension) 30); i++;

    scrollV = XtCreateManagedWidget("scrollV", scrollbarWidgetClass, form,
				 arg, (Cardinal)i);

    XtOverrideTranslations(scrollV, XtParseTranslationTable("<Btn2Up>: NotifyScroll(Proportional) NotifyEndThumb() EndScroll()"));

    XawScrollbarSetThumb(scrollV, (180.0 - surface_rot_x) / 180.0, -1.0);
    XtAddCallback(scrollV, XtNjumpProc, gotJumpV, (XtPointer) infoV);
    XtAddCallback(scrollV, XtNscrollProc, gotScrollV, (XtPointer) infoV);
    XtAddCallback(scrollV, XtNscrollProc, reset_rotate, (XtPointer) infoV);
    return mainWin;
}



/*********************************************************************
*
* The following are callback handlers for scroll bars and the menu.
*
*********************************************************************/

void gotScrollV(Widget scrollbar, XtPointer client_data, XtPointer position)
{
    Arg arg[1];
    static char line[80];
    int height;

    XtSetArg(arg[0], XtNheight, &height);
    XtGetValues(scrollbar, arg, (Cardinal)1);

    if ((int)position) {
	surface_rot_x += (((int)position)>0)? -10: 10;
	
	if (surface_rot_x > 180)
		surface_rot_x = 180;
	else if (surface_rot_x < 0)
		surface_rot_x = 0;

    	sprintf(line, INFOV_FORMAT, surface_rot_x);

        XtSetArg(arg[0], XtNlabel, line);
        XtSetValues((Widget)client_data, arg, (Cardinal)1);

        XawScrollbarSetThumb(scrollbar, (180.0 - surface_rot_x)/180.0, -1.0);

        displayScreen(SA_JUMP);
    }
}


void gotJumpV(Widget scrollbar, XtPointer client_data, XtPointer position)
{
    Arg arg[1];
    static char line[80];

    surface_rot_x = (1.0 - *((float *)position)) * 180.0;
    sprintf(line, INFOV_FORMAT, surface_rot_x);

    XtSetArg(arg[0], XtNlabel, line);
    XtSetValues((Widget)client_data, arg, (Cardinal)1);

    displayScreen(SA_ROTATE);
}



void gotScrollH(Widget scrollbar, XtPointer client_data, XtPointer position)
{
    Arg arg[1];
    static char line[80];
   
    surface_rot_z += (((int)position)>0)? -20: 20;
	
    if (surface_rot_z > 360)
        surface_rot_z = 360;
    else if (surface_rot_z < 0)
        surface_rot_z = 0;

    sprintf(line, INFOH_FORMAT, surface_rot_z);

    XtSetArg(arg[0], XtNlabel, line);
    XtSetValues((Widget)client_data, arg, (Cardinal)1);

    XawScrollbarSetThumb(scrollbar, surface_rot_z/360.0, -1.0);

    displayScreen(SA_JUMP);
}


void gotJumpH(Widget scrollbar, XtPointer client_data, XtPointer position)
{
    Arg arg[1];
    static char line[80];

    surface_rot_z = *((float *)position) * 360.0;
    sprintf(line, INFOH_FORMAT, surface_rot_z);

    XtSetArg(arg[0], XtNlabel, line);
    XtSetValues((Widget)client_data, arg, (Cardinal)1);

    displayScreen(SA_ROTATE);
}


void plot_smooth()
{
   DestroyMenu(viewmenu); 

   viewmenu = CreateMenu(viewmenuTitle, VIEWMENU_TITLE, viewSmoothMenu, NULL);
   
   displayScreen(SA_SMOOTH);
}

void plot_unsmooth()
{

   DestroyMenu(viewmenu);

   viewmenu = CreateMenu(viewmenuTitle, VIEWMENU_TITLE, viewRawMenu, NULL);

   displayScreen(SA_ROUGH);
}


void plot_usemed()
{
   DestroyMenu(viewmenu); 

   /* Update memu for the new alternative */
   viewSmoothMenu[MED_POS].name = AVG_MSG;
   viewSmoothMenu[MED_POS].func = (void *) plot_nomed;
   
   viewmenu = CreateMenu(viewmenuTitle, VIEWMENU_TITLE, viewSmoothMenu, NULL);
   
   displayScreen(SA_USEMED);
}


void plot_nomed()
{

   DestroyMenu(viewmenu);

   /* Update menu for the new alternative */
   viewSmoothMenu[MED_POS].name = MED_MSG;
   viewSmoothMenu[MED_POS].func = (void *) plot_usemed;

   viewmenu = CreateMenu(viewmenuTitle, VIEWMENU_TITLE, viewSmoothMenu, NULL);

   displayScreen(SA_NOMED);
}

void reset_rotate()
{
   surface_rot_z = 30.0;
   surface_rot_x = 60.0;

   XawScrollbarSetThumb(scrollV, (180.0 - surface_rot_x)/180.0, -1.0);
   XawScrollbarSetThumb(scrollH, surface_rot_z/360.0, -1.0);
   displayScreen(SA_JUMP);
}
