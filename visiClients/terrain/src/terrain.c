/*-----------------------------------------------------------------------------
 *   gnuplot_x11 - X11 outboard terminal driver for gnuplot 2
 *
 *   Requires installation of companion inboard x11 driver in gnuplot/term.c
 *
 *   Acknowledgements: 
 *      Chris Peterson (MIT) - original Xlib gnuplot support (and Xaw examples)
 *      Dana Chee (Bellcore)  - mods to original support for gnuplot 2.0
 *      Arthur Smith (Cornell) - graphical-label-widget idea (xplot)
 *      Hendri Hondorp (University of Twente, The Netherlands) - Motif xgnuplot
 *
 *   This code is provided as is and with no warranties of any kind.
 *       
 *   Ed Kubaitis - Computing Services Office -  University of Illinois, Urbana
 *---------------------------------------------------------------------------*/

/*
 * Modified for terrain plot:
 *      Chi-Ting Lam
 *
 * terrain.c - main entry point and x driver.
 *
 * $Id: terrain.c,v 1.15 2000/08/21 00:32:31 paradyn Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <assert.h>
#include <string.h>

extern /*"C"*/ const char V_terrain[];
extern /*"C"*/ const char V_libpdutil[];
extern /*"C"*/ const char V_libvisi[];

#define Ncolors 11		/* Number of colors for GNUPlot part */

#include "terrain.h"
#include "term.h"
#include "form.h"
#include "command.h"
#include "misc.h"
#include "setshow.h"
#include "pdutil/h/pdsocket.h"
#include "visi/h/visualization.h" 

unsigned long colors[Ncolors];	/* Colors for GNUPlot part */
char color_keys[Ncolors][30] =   { "text", "border", "axis", 
   "line1", "line2", "line3", "line4", "line5", "line6", "line7", "line8" };
char color_values[Ncolors][30] = { "black", "black", "black", 
   "red",  "black", "blue",  "magenta", "cyan", "sienna", "orange", "coral" };

int color_disp = 0;		/* Do we have a color display? */
int Ntcolors=25;		/* Number of color used by terrain */
unsigned long *tcolors;		/* The colors used by terrain */

/* Line stypes */

char dashes[10][5] = { {0}, {1,6,0}, 
   {0}, {4,2,0}, {1,3,0}, {4,4,0}, {1,5,0}, {4,4,4,1,0}, {4,2,0}, {1,3,0}
   };

Widget w_top,			/* Top level application widget */
       w_label;			/* The main viewer window */
Window win;			/* Which window we are in */
Display *dpy;			/* Which display we are in */
Pixmap pixmap;			/* The drawing area in the main viewer */
GC gc = (GC)NULL;
Dimension W = 700, H = 450, D = 0; /* Initial dimension of terrain window */
Arg args[5];

int cx=0, cy=0, vchar, nc = 0;	/* Char location, size, length, etc */

int cur_lt;			/* What kind of line we are using */

float xscale, yscale;		/* GNUPlot rescale the plot to fit the */
				/* window before actually drawing      */
enum JUSTIFY jmode;		/* How texts are drawn */

RValues rv;

XtResource resources[] = {
   { XtNfont, XtCFont, XtRFontStruct, sizeof(XFontStruct *), 
     XtOffset(RVptr, font), XtRString, "fixed" },
   { XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel), 
     XtOffset(RVptr, fg), XtRString, "black" },
   { XtNbackground, XtCBackground, XtRPixel, sizeof(Pixel), 
     XtOffset(RVptr, bg), XtRString, "white" },
   };

/* New action to tell terrain to draw shadow plot when rotating */
/* XtActionProc NotifyEndThumb(); */

XtActionsRec actionTable[] = {
   { "NotifyEndThumb", NotifyEndThumb }
};

int display(int action); 


/***********************************************************************************
 ************************* modified section starts *********************************/

/*
 * default resources
 */

String fallback_resources[] = { 
 "*background: Grey",
 "*foreground: Black",
 "*font: *-Helvetica-*-r-*-12-*",
 "*titlebar*font: *-New*Century*Schoolbook-Bold-R-*-18-*",
 "*titlebar*background: Red",
 "*titlebar*foreground: White",
 "*logo*background: White",
 "*logo*foreground: #c06077",
 "*title*font: *-New*Century*Schoolbook-*-R-*-14-*",
 NULL,
};

static int numGroups;
static int phase_displayed;
static Widget input;
static int numRes;

struct HistData {
    int      curve_id;
    unsigned prev_last;
    char*    title;
    unsigned metric;
    unsigned resource;
    int      groupId;
};

static int get_groupId(const char *axis_label){

  int id = -1;
  struct HistData* hdp = 0;
  const char *temp;
  int m, r;

  for (m = 0; m < 1; m++) {
     temp = visi_MetricLabel(m);
     for (r = 0; r < numRes; r++) {
        if(visi_Enabled(m,r)){
           if(strcmp(temp,axis_label) == 0){
              hdp = (struct HistData *)visi_GetUserData(m,r);
              if(hdp){
                id =  hdp->groupId;
                break;
              }
           }
        }
  }}

  if(id == -1){         /* create a new group number */
    id = numGroups++;
  }

  return(id);

}


static int add_new_curve(unsigned m, unsigned r)
{
  struct HistData* hdp = 0;
  char* m_name;
  char* r_name;
  char* p_name;
  char* label_name;

    if (visi_Enabled((signed)m,(signed)r) &&
       (hdp = (struct HistData *) visi_GetUserData((signed)m,(signed)r)) == 0) {

        hdp = (struct HistData *) malloc(sizeof(struct HistData));
        assert(hdp);

        hdp->curve_id  = -1;
        hdp->groupId  = -1;
        hdp->prev_last = 0;
        hdp->metric    = m;
        hdp->resource  = r;

        m_name = visi_MetricName((signed)m);
        r_name = visi_ResourceName((signed)r);
	p_name = visi_GetMyPhaseName();
	label_name = visi_MetricLabel((signed)m);
        m_name = (m_name) ? (m_name) : "";
        r_name = (r_name) ? (r_name) : "";

        hdp->title   = (char *) malloc(strlen(m_name)+strlen(r_name)+4);
        assert(hdp->title);
        sprintf(hdp->title, "%s <%s>", m_name, r_name);


        hdp->groupId = get_groupId(visi_MetricLabel((signed)m));

        hdp->curve_id = Graph3DAddNewCurve(m_name, r_name, p_name, 
					   label_name,  
					   visi_NumBuckets(), numRes);

        visi_SetUserData((signed)m,(signed)r,(void *) hdp);

   }
    
   return 0;



}





static void drawData(int is_fold)
{
    struct HistData* hdp;
    int m, r;
    float *data;

    for (m = 0; m < 1; m++) {
        numRes = visi_NumResources();

        for (r = 0; r < numRes; r++) {
         if (!visi_Valid(m,r)) {  /* invalid histo-cell */
                continue;
         }
            /* this curve was previously deleted and now has new data
             * need to add new curve to histogram widget */
         if((hdp = (struct HistData *) visi_GetUserData((signed)m,(signed)r)) == 0){
                add_new_curve((unsigned)m, (unsigned)r);
                hdp = (struct HistData *) visi_GetUserData((signed)m,(signed)r);
                assert(hdp);
         }
         
	 data = visi_DataValues(m,r);

	    /* TODO: set start time to real start time rather than 0.0 */
	    /* HistSampleInfo(hw, BucketWidth(), NumBuckets(), 0.0, FALSE); */

	 if (is_fold) {
		Graph3DSetCurveData(hdp->curve_id,0,
				 visi_LastBucketFilled(m,r),data, visi_GetStartTime(), 
				 visi_BucketWidth(), is_fold,
				 color_disp, dpy, gc, rv, pixmap, W, H, win);
  	 } 
         else {

		Graph3DSetCurveData(hdp->curve_id, 
				    hdp->prev_last,
				    visi_LastBucketFilled(m,r) - hdp->prev_last,
				    &(data[hdp->prev_last]), visi_GetStartTime(), 
				    visi_BucketWidth(), is_fold,
				    color_disp, dpy, gc, rv, pixmap, W, H, win);
	
         }
	 hdp->prev_last = visi_LastBucketFilled(m,r);
       }
     }

      
   return;



  
}


/* Event Handler to process new values from the visilib */
static int process_datavalues(int parameter)
{
  static int checkError = 1;
  
  /* check for errors for datas come in at the first time */
  if (checkError == 1)  
  {
     if (visi_NumMetrics() != 1)
        visi_showErrorVisiCallback("Please select only one metric for the 3D Histogram.\nOnly one of the metrics selected will be shown.");

     if (visi_NumResources() < 2)
         visi_showErrorVisiCallback("Please select more than one resource for the 3D Histogram.\nNo curve will be shown.");

     if (visi_NumResources() > 15)
        visi_showErrorVisiCallback("It's advised to have fewer than 15 curves.");

     checkError = 0;
  }

  /* get and draw the graph */
  drawData(0);

  return 0;

}



/* Event Handler to process new values from the visilib */
static int process_fold(int parameter)
{

   /* get and draw the graph */
   drawData(1);

   
   return 0;

}

/**************************modified section ends ************************************
 ************************************************************************************/

/*-----------------------------------------------------------------------------
 *   display - display accumulated commands from inboard driver
 *---------------------------------------------------------------------------*/
void displayScreen(int action)
{
    display(action);
}


int display(int action)
{
   /* clean the screen when ReDisplay the Graph is needed */
   #ifndef MOTIF
   if (color_disp) { /* Athena needs different erase for color and mono */
   #endif
      XSetForeground(dpy, gc, rv.bg);
      XFillRectangle(dpy, win, gc, 0, 0, W, H);
      XFillRectangle(dpy, pixmap, gc, 0, 0, W, H);
      XSetForeground(dpy, gc, rv.fg);
      XSetBackground(dpy, gc, rv.bg);
   #ifndef MOTIF
   }
   else {
      XSetFunction(dpy, gc, GXxor);
      XCopyArea(dpy, win, win, gc, 0, 0, W, H, 0, 0);
      XSetFunction(dpy, gc, GXcopyInverted);
   }
   #endif


   /* get and draw the graph */
   plot3drequest(action); 

   /* trigger expose events to display pixmap */
   XClearArea(dpy, win, 0, 0, 0, 0, True);

   return 0;
}



/*************************************************************************
* resize - Called by X when the window got resized.  It make a new pixmap
*          according to the new size of the main viewer, and redraw the
*          terrain in it.
*
*************************************************************************/


static void
resize(w, cd, e) 
Widget w;
char *cd;
XConfigureEvent *e; {
   if (e->type != ConfigureNotify) return;
   W = e->width; H = e->height;

   init_pixmap();

   display(SA_RESIZE); 


}


/*************************************************************************
*
* init_pixmap - Create a pixmap corresponding to the current size of the
*               main viewer.
*
*************************************************************************/

int init_pixmap()
{
   /* set scaling factor between internal driver & window geometry */
   xscale = (float)W / 4096.;  yscale = (float)H / 4096.;  

   /* create new pixmap & GC */
   if (gc) { XFreeGC(dpy, gc); XFreePixmap(dpy, pixmap); }
   pixmap = XCreatePixmap(dpy, RootWindow(dpy,DefaultScreen(dpy)), W, H, D);
   gc = XCreateGC(dpy, win, 0, NULL);
   XSetFont(dpy, gc, rv.font->fid);

   /* the display belongs to w_label */
   XtSetArg(args[0], XtNbitmap, pixmap);
   XtSetValues(w_label, args, (Cardinal)1);


   return 0;
}




/*************************************************************************
*
* X11_vector - used by GNUPlot to draw axis and the base lines.
*
*************************************************************************/

int X11_vector(unsigned int x, unsigned int y)
{
      XDrawLine(dpy, win, gc, X(cx), Y(cy), X(x), Y(y));

   XDrawLine(dpy, pixmap, gc, X(cx), Y(cy), X(x), Y(y));
   cx = x; cy = y;

  return 0;
}


/*************************************************************************
*
* X11_move - used by GNUPlot to draw axis and the base lines.
*
*************************************************************************/

int X11_move(unsigned int x, unsigned int y)
{
   cx = x; cy = y;

   return 0;
}



/*************************************************************************
*
* X11_put_text - put a string to the specified location in the main viewer
*
*************************************************************************/

int X11_put_text(unsigned int x, unsigned int y, char *str)
{
   int sw, sl;
   sl = strlen(str);
   sw = XTextWidth(rv.font, str, sl);

   switch(jmode) {
      case LEFT:   sw = 0;     break;
      case CENTRE: sw = -sw/2; break;
      case RIGHT:  sw = -sw;   break;
   }

   if (!color_disp) 
   {
	 XDrawString(dpy, win, gc, X(x)+sw, Y(y)+vchar/3, str, sl);

      XDrawString(dpy, pixmap, gc, X(x)+sw, Y(y)+vchar/3, str, sl);
   }
   else { 
      XSetForeground(dpy, gc, colors[0]);
	 XDrawString(dpy, win, gc, X(x)+sw, Y(y)+vchar/3, str, sl);

      XDrawString(dpy, pixmap, gc, X(x)+sw, Y(y)+vchar/3, str, sl);
      XSetForeground(dpy, gc, colors[cur_lt+1]);
   }

   return 0;
}



/*************************************************************************
*
* X11_justify_text - Set the justify (left, right, middle) mode when
*                    drawing text.
*
*************************************************************************/

int X11_justify_text(enum JUSTIFY mode)
{
   jmode = mode;
   return 1;
}




/*************************************************************************
*
* X11_linetype - Set the current line type.
*
*************************************************************************/

int X11_linetype(int lt)
{ 
   int width, type;

    lt = (lt+2)%10;
    width = (lt == 0) ? 2 : 0;
    if (color_disp) {
        if (lt != 1) 
            type = LineSolid;
        else {
            type = LineOnOffDash;
            XSetDashes(dpy, gc, 0, dashes[lt], (signed)strlen(dashes[lt]));
        }
        XSetForeground(dpy, gc, colors[lt+1]);
    } else {
        type  = (lt == 0 || lt == 2) ? LineSolid : LineOnOffDash;
	if (dashes[lt][0])
	    XSetDashes(dpy, gc, 0, dashes[lt], (signed)strlen(dashes[lt]));
    }
    XSetLineAttributes( dpy,gc, (unsigned)width, type, CapButt, JoinBevel);

    cur_lt = lt;

    return 0;
}





/*************************************************************************
*
* hsv2Rgb - convert a color in HSV system into RGB system.
*           X11R5 should do a better job than this, but R4 does not even
*           have this kind of functions.
*           The algorithm can be found in many graphic text books.
*
*************************************************************************/


int hsv2Rgb(hue, val, sat, rgb)
float hue, val, sat;
XColor *rgb;
{
   float minCol;

   hue = (hue/360.0 - ((int)hue)/360)*360.0;
   minCol = val * (1.0 - sat);
   if (hue <= 120.0) {
      rgb->green = minCol;
      if (hue <= 60.0) {
	 rgb->red = val;
	 rgb->blue = minCol + hue * (val - minCol)/(120.0 - hue);
      } else {
	 rgb->blue = val;
	 rgb->red = minCol + (120.0 - hue) * (val - minCol)/hue;
      }
   } else if (hue <= 240.0) {
      rgb->red = minCol;
      if (hue <= 180.0) {
	 rgb->blue = val;
	 rgb->green = minCol + (hue - 120.0) * (val - minCol)/(240 - hue);
      } else {
	 rgb->green = val;
	 rgb->blue = minCol + (240.0 - hue) * (val - minCol)/(hue - 120.0);
      }
   } else {
      rgb->blue = minCol;
      if (hue <= 300.0) {
         rgb->green = val;
	 rgb->red = minCol + (hue - 240.0) * (val - minCol)/(360.0 - hue);
      } else {
	 rgb->red = val;
	 rgb->green = minCol + (360.0 - hue) * (val - minCol)/(hue - 240.0);
      }
   }

   return 0;

}




/*************************************************************************
*
* NotifyEndThumb - An application action which enable terrain to tell the
*                  difference between jumping and smooth rotating.
*                  (Xaw does not have enough action to do this)
*
*************************************************************************/


XtActionProc
NotifyEndThumb(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
   display(SA_JUMP);
}


/*************************************************************************
* The following dummy routines keeps GNUPlot happy
*************************************************************************/

int X11_init()
{
  return 0;
}

int X11_reset()
{
  return 0;
}

int X11_graphics()
{
  return 0;
}

int X11_text()
{
  return 0;
}




/*-----------------------------------------------------------------------------
 *   main program - fire up application and callbacks
 *---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
   float colStep;

   XtAppContext app_con;	/* Application context */ 

/*****************************************************************************
 ******************* modified section starts *********************************/
 
   int fd;
   int i;

   outfile = stdout; /* non-static initialization */

   for (i=1; i<argc; i++)
     {
       if (strcmp(argv[i],"-V")==0)
          fprintf(stderr,"%s\n",V_terrain);
     }

   fd = visi_Init();
   if (fd < 0)
   {
      fprintf(stderr, "error initializing Visi library\n");
      exit(-1);
   }
   
   phase_displayed = 0;
   numGroups = 0;

/**************************modified section ends ************************************
 ************************************************************************************/

   signal(SIGINT, SIG_IGN);
   signal(SIGTSTP, SIG_IGN);



   /* initialize application */
   w_top = XtAppInitialize(&app_con, "Terrain", NULL, ZERO, &argc, argv,
			   fallback_resources, NULL, ZERO);

   /* Initialize application action for scroll bar jump clean-ups */
   XtAppAddActions(app_con, actionTable, ONE);

   /* Initialize the frame */
   w_label = (Widget) createForm(w_top, H, W);

   tcolors = (unsigned long*)terrain_alloc(sizeof(unsigned long) * Ntcolors);

   /* Don't display the window until it is ready */
   XtSetMappedWhenManaged(w_top, False);
   XtRealizeWidget(w_top);


   /* extract needed information */
   dpy = XtDisplay(w_top); win = XtWindow(w_label);

   /* Do we have enough colors? */
   color_disp = (XDisplayCells(dpy, DefaultScreen(dpy)) > 40);
   if (color_disp)
       D = DefaultDepth(dpy, DefaultScreen(dpy));
   else
       D = 1;

   if (color_disp) {
      char option[20], *value; 
      XColor used, exact; 
      int n;

      /* Allocate colors for old GNUPlot stuffs */
      for(n=0; n<Ncolors; n++) {
	 strcpy(option, color_keys[n]);
	 strcat(option, "Color");
	 value = XGetDefault(dpy, "terrain", option);
	 if (!value) { value = color_values[n]; }
	 if (XAllocNamedColor(dpy, DefaultColormap(dpy,0), value, &used,&exact))
	    colors[n] = used.pixel; 
	 else {
	    fprintf(stderr, "terrain: cannot allocate %s:%s\n", option, value);
	    fprintf(stderr, "terrain: assuming %s:black\n", option);
	    colors[n] = BlackPixel(dpy,0);
	 }
      }

      /* Allocate a spectum of colors for Terrain Plot */

      colStep = 330.0/(float) Ntcolors;

      for(n=0; n<Ntcolors; n++) {
	hsv2Rgb(((float) n)*colStep+75.0, 65535.0, 0.75, &used);

	if (XAllocColor(dpy, DefaultColormap(dpy,0), &used)) {
	   tcolors[n] = used.pixel;
/*
	   fprintf(stderr, "R=%u, G=%u, B=%u\n", used.red, used.green, used.blue);
*/
	} else {
	   fprintf(stderr, "Warning: cannot allocate color R=%u, B=%u\n", used.red, used.blue);
	   tcolors[n] = WhitePixel(dpy,0);
        }
      }
   }

   XtSetArg(args[0], XtNwidth, &W);
   XtSetArg(args[1], XtNheight,&H);
   XtGetValues(w_label, args, (Cardinal)2);
   XtGetApplicationResources(w_top, &rv, resources, XtNumber(resources),NULL,0);
   vchar = (rv.font->ascent + rv.font->descent);

   /* add callbacks on window-resized */
   XtAddEventHandler(w_label, StructureNotifyMask, FALSE, (XtEventHandler)resize,
		     NULL);
   


/***********************************************************************************
 ************************* modified section starts *********************************/
  
  (void) visi_RegistrationCallback(DATAVALUES, process_datavalues);
  (void) visi_RegistrationCallback(FOLD,       process_fold);

  XtAppAddInput(app_con, fd, (XtPointer)XtInputReadMask,
		(XtInputCallbackProc)visi_callback,
		(XtPointer)input);


/**************************modified section ends ************************************
 ************************************************************************************/

   /* Initialize terminal */
   change_term("x11", 3);

   /* PopUpInit(); */
   

   /* Create and display the graph */
   init_pixmap();

   /* Clean the screen when starts up */
   #ifndef MOTIF
   if (color_disp) { /* Athena needs different erase for color and mono */
   #endif
      XSetForeground(dpy, gc, rv.bg);
      XFillRectangle(dpy, win, gc, 0, 0, W, H);
      XFillRectangle(dpy, pixmap, gc, 0, 0, W, H);
      XSetForeground(dpy, gc, rv.fg);
      XSetBackground(dpy, gc, rv.bg);
   #ifndef MOTIF  
      }
   else {
      XSetFunction(dpy, gc, GXxor);
      XCopyArea(dpy, win, win, gc, 0, 0, W, H, 0, 0);
      XCopyArea(dpy, pixmap, pixmap, gc, 0, 0, W, H, 0, 0);
      XSetFunction(dpy, gc, GXcopyInverted);
      }
   #endif

   XtMapWidget(w_top);

   XtAppMainLoop(app_con);

   return 0;
}

