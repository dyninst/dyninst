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
 */

/*
 * terrain.h - header file of terrain.c.
 *
 * $Log: terrain.h,v $
 * Revision 1.5  1997/05/22 02:18:27  tung
 * Revised.
 *
 * Revision 1.4  1997/05/21 21:14:34  tung
 * No restriction on number of resources but a warning message if numRes > 15.
 *
 * Revision 1.3  1997/05/21 02:27:31  tung
 * Revised.
 *
 * Revision 1.2  1997/05/20 08:29:21  tung
 * Revised on resizing the maxZ, change the xlabel and zlabel format.
 *
 * Revision 1.1  1997/05/12 20:15:45  naim
 * Adding "Terrain" visualization to paradyn (commited by naim, done by tung).
 *
 * Revision 1.2  1992/05/19  17:40:09  lam
 * Added definition of win which pop up needs for initializing
 * its location.
 *
 * Revision 1.1  1992/05/19  07:21:56  lam
 * Initial revision
 *
 *
 */

#ifndef TERRAIN_H
#define TERRAIN_H

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Cardinals.h>

#include <Label.h>          /* use -Idir for location on your system */

#include "misc.h"

#ifdef MOTIF
#include <Xm.h>             /* use -Idir for location on your system */
#define LabelWC xmLabelWidgetClass
#define LabelBPM XmNbackgroundPixmap
#else
#define LabelWC labelWidgetClass
#define LabelBPM XtNbitmap
#endif

extern void displayScreen(int action);
extern int X11_vector(unsigned int x, unsigned int y);
extern int X11_move(unsigned int x, unsigned int y);
extern int X11_put_text(unsigned int x, unsigned int y, char *str);
extern int X11_justify_text(enum JUSTIFY mode);
extern int X11_linetype(int lt);
extern X11_colorPoly();
extern X11_init();
extern X11_reset();
extern X11_graphics();
extern X11_text();
extern X11_colorPoly_pixmap();





typedef struct {       /* See "X Toolkit Intrinsics Programming Manual"      */
  XFontStruct *font;   /* Nye and O'Reilly, O'Reilly & Associates, pp. 80-85 */
  unsigned long fg;
  unsigned long bg;
  } RValues, *RVptr; 

extern RValues rv;

extern int color_disp;     /* Do we have a color display? */

extern float xscale, yscale;
extern Display *dpy;
extern GC gc;
extern Pixmap pixmap;
extern cur_lt;
extern unsigned long colors[];
extern int Ntcolors;
extern unsigned long *tcolors;
extern Widget w_top;
extern Window win;

extern vchar;			/* Height of the default font */



int get_groupId(const char *axis_label);
int add_new_curve(unsigned m, unsigned r);
void drawData(int is_fold);
int process_datavalues(int parameter);
int process_fold(int parameter);

void resize(Widget w, char *cd, XConfigureEvent *e);

/* New action to tell terrain to draw shadow plot when rotating */
XtActionProc NotifyEndThumb();


//int display(int action);
int init_pixmap();






#define X(x) (Dimension) (x * xscale)
#define Y(y) (Dimension) ((4095-y) * yscale)

/* Draw a polygon formed by 4 points */

#define X11_poly(x1, y1, x2, y2, x3, y3, x4, y4) \
{\
   XPoint pts[5];\
\
   pts[0].x = X(x1); pts[0].y = Y(y1);\
   pts[1].x = X(x2); pts[1].y = Y(y2);\
   pts[2].x = X(x3); pts[2].y = Y(y3);\
   pts[3].x = X(x4); pts[3].y = Y(y4);\
   pts[4].x = X(x1); pts[4].y = Y(y1);\
\
   XSetForeground(dpy, gc, colors[cur_lt+1]);\
   XDrawLines(dpy, pixmap, gc, pts, 5, CoordModeOrigin);\
\
}



/* Draw and fill a polygon formed by 4 points in back and white */

#define X11_monoPoly(x1, y1, x2, y2, x3, y3, x4, y4) \
{\
   XPoint pts[5];\
\
   pts[0].x = X(x1); pts[0].y = Y(y1);\
   pts[1].x = X(x2); pts[1].y = Y(y2);\
   pts[2].x = X(x3); pts[2].y = Y(y3);\
   pts[3].x = X(x4); pts[3].y = Y(y4);\
   pts[4].x = X(x1); pts[4].y = Y(y1);\
\
   XSetForeground(dpy, gc, rv.bg);\
   XFillPolygon(dpy, pixmap, gc, pts, 5, Complex, CoordModeOrigin);\
\
   XSetForeground(dpy, gc, colors[cur_lt+1]);\
   XDrawLines(dpy, pixmap, gc, pts, 5, CoordModeOrigin);\
\
}\


/* Draw and fill a polygon formed by "size" points in the color specified */

#define PTS_SIZE 10

#define X11_colorPoly(ptLst, size,  color)\
{\
   XPoint ptsMem[PTS_SIZE];\
   XPoint *pts;\
   int i, result;\
\
   if (size < (PTS_SIZE - 1)) {\
        pts = ptsMem;\
  }  else {\
      if ((pts=(XPoint *)terrain_alloc(sizeof(XPoint)*(size+1))) == NULL) {\
         printf("memory allocation error");\
         exit(-1);\
      }\
   }\
\
   for(i=0; i<size; i++) {\
	pts[i].x = X(ptLst[i<<1]);\
	pts[i].y = Y(ptLst[(i<<1)+1]);\
   }\
\
   pts[size].x = X(ptLst[0]);\
   pts[size].y = Y(ptLst[1]);\
\
   XSetForeground(dpy, gc, tcolors[color%Ntcolors]);\
   XFillPolygon(dpy, pixmap, gc, pts, size+1, Complex, CoordModeOrigin);\
   XSetForeground(dpy, gc, colors[cur_lt+1]);\
\
   if (pts!=ptsMem)\
      free(pts);\
\
}






/* Draw a polygon formed by 4 points */

#define X11_poly_window(x1, y1, x2, y2, x3, y3, x4, y4) \
{\
   XPoint pts[5];\
\
   pts[0].x = X(x1); pts[0].y = Y(y1);\
   pts[1].x = X(x2); pts[1].y = Y(y2);\
   pts[2].x = X(x3); pts[2].y = Y(y3);\
   pts[3].x = X(x4); pts[3].y = Y(y4);\
   pts[4].x = X(x1); pts[4].y = Y(y1);\
\
   XSetForeground(dpy, gc, colors[cur_lt+1]);\
   XDrawLines(dpy, win, gc, pts, 5, CoordModeOrigin);\
\
}



/* Draw and fill a polygon formed by 4 points in back and white */

#define X11_monoPoly_window(x1, y1, x2, y2, x3, y3, x4, y4) \
{\
   XPoint pts[5];\
\
   pts[0].x = X(x1); pts[0].y = Y(y1);\
   pts[1].x = X(x2); pts[1].y = Y(y2);\
   pts[2].x = X(x3); pts[2].y = Y(y3);\
   pts[3].x = X(x4); pts[3].y = Y(y4);\
   pts[4].x = X(x1); pts[4].y = Y(y1);\
\
   XSetForeground(dpy, gc, rv.bg);\
   XFillPolygon(dpy, win, gc, pts, 5, Complex, CoordModeOrigin);\
\
   XSetForeground(dpy, gc, colors[cur_lt+1]);\
   XDrawLines(dpy, win, gc, pts, 5, CoordModeOrigin);\
\
}\


/* Draw and fill a polygon formed by "size" points in the color specified */

#define X11_colorPoly_window(ptLst, size,  color)\
{\
   XPoint ptsMem[PTS_SIZE];\
   XPoint *pts;\
   int i, result;\
\
   if (size < (PTS_SIZE - 1)) {\
        pts = ptsMem;\
  }  else {\
      if ((pts=(XPoint *)terrain_alloc(sizeof(XPoint)*(size+1))) == NULL) {\
         printf("memory allocation error");\
         exit(-1);\
      }\
   }\
\
   for(i=0; i<size; i++) {\
	pts[i].x = X(ptLst[i<<1]);\
	pts[i].y = Y(ptLst[(i<<1)+1]);\
   }\
\
   pts[size].x = X(ptLst[0]);\
   pts[size].y = Y(ptLst[1]);\
\
   XSetForeground(dpy, gc, tcolors[color%Ntcolors]);\
   XFillPolygon(dpy, win, gc, pts, size+1, Complex, CoordModeOrigin);\
   XSetForeground(dpy, gc, colors[cur_lt+1]);\
\
   if (pts!=ptsMem)\
      free(pts);\
\
}







#endif
