/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

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
 * terrain.h - header file of terrain.c.
 *
 * $Id: terrain.h,v 1.8 2004/03/23 19:14:00 eli Exp $
 */

#ifndef TERRAIN_H
#define TERRAIN_H

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Cardinals.h>

#include <X11/Xaw/Label.h>

#include "misc.h"

#ifdef MOTIF
#include <Xm.h>             /* use -Idir for location on your system */
#define LabelWC xmLabelWidgetClass
#define LabelBPM XmNbackgroundPixmap
#else
#define LabelWC labelWidgetClass
#define LabelBPM XtNbitmap
#endif

#include "plot.h"

extern void displayScreen(int action);
extern int X11_vector(unsigned int x, unsigned int y);
extern int X11_move(unsigned int x, unsigned int y);
extern int X11_put_text(unsigned int x, unsigned int y, char *str);
extern int X11_justify_text(enum JUSTIFY mode);
extern int X11_linetype(int lt);
/* extern X11_colorPoly(); */
extern int X11_init(void);
extern int X11_reset(void);
extern int X11_graphics(void);
extern int X11_text(void);
/* extern X11_colorPoly_pi[xmap(); */





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
extern int cur_lt;
extern unsigned long colors[];
extern int Ntcolors;
extern unsigned long *tcolors;
extern Widget w_top;
extern Window win;

extern int vchar;			/* Height of the default font */



int get_groupId(const char *axis_label);
int add_new_curve(unsigned m, unsigned r);
void drawData(int is_fold);
int process_datavalues(int parameter);
int process_fold(int parameter);

void resize(Widget w, char *cd, XConfigureEvent *e);

/* New action to tell terrain to draw shadow plot when rotating */
XtActionProc NotifyEndThumb();


/*int display(int action); */
int init_pixmap(void);






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
