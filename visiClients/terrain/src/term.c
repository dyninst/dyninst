/* GNUPLOT - term.c */
/*
 * Copyright (C) 1986, 1987, 1990   Thomas Williams, Colin Kelley
 *
 * Permission to use, copy, and distribute this software and its
 * documentation for any purpose with or without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and 
 * that both that copyright notice and this permission notice appear 
 * in supporting documentation.
 *
 * Permission to modify the software is granted, but not the right to
 * distribute the modified code.  Modifications are to be distributed 
 * as patches to released version.
 *  
 * This software  is provided "as is" without express or implied warranty.
 * 
 *
 * AUTHORS
 * 
 *   Original Software:
 *     Thomas Williams,  Colin Kelley.
 * 
 *   Gnuplot 2.0 additions:
 *       Russell Lang, Dave Kotz, John Campbell.
 * 
 * send your comments or suggestions to (pixar!info-gnuplot@sun.com).
 * 
 */

/*
 * Adapted for terrain plot:
 *      Chi-Ting Lam
 *
 */

#ifndef lint
static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/visiClients/terrain/src/term.c,v 1.1 1997/05/12 20:15:43 naim Exp $";
#endif

/*
 * term.c - misc terminal routines.
 *
 * $Log: term.c,v $
 * Revision 1.1  1997/05/12 20:15:43  naim
 * Adding "Terrain" visualization to paradyn (commited by naim, done by tung).
 *
 * Revision 1.1  1992/05/19  06:30:55  lam
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include "plot.h"
#include "setshow.h"
#include "term.h"

#include "terrain.h"

/* for use by all drivers */
#define sign(x) ((x) >= 0 ? 1 : -1)
#define abs(x) ((x) >= 0 ? (x) : -(x))
#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

BOOLEAN term_init;			/* true if terminal has been initialized */

extern FILE *outfile;
extern char outstr[];
extern BOOLEAN term_init;
extern int term;
extern float xsize, ysize;

extern char input_line[];
extern struct lexical_unit token[];

/* This is needed because the unixplot library only writes to stdout. */
int unixplot=0;

#define NICE_LINE		0
#define POINT_TYPES		6


do_point(x,y,number)
int x,y;
int number;
{
register int htic,vtic;
register struct termentry *t = &term_tbl[term];

     if (number < 0) {		/* do dot */
	    (*t->move)(x,y);
	    (*t->vector)(x,y);
	    return;
	}

	number %= POINT_TYPES;
	htic = (t->h_tic/2);	/* should be in term_tbl[] in later version */
	vtic = (t->v_tic/2);	

	switch(number) {
		case 0: /* do diamond */ 
				(*t->move)(x-htic,y);
				(*t->vector)(x,y-vtic);
				(*t->vector)(x+htic,y);
				(*t->vector)(x,y+vtic);
				(*t->vector)(x-htic,y);
				(*t->move)(x,y);
				(*t->vector)(x,y);
				break;
		case 1: /* do plus */ 
				(*t->move)(x-htic,y);
				(*t->vector)(x-htic,y);
				(*t->vector)(x+htic,y);
				(*t->move)(x,y-vtic);
				(*t->vector)(x,y-vtic);
				(*t->vector)(x,y+vtic);
				break;
		case 2: /* do box */ 
				(*t->move)(x-htic,y-vtic);
				(*t->vector)(x-htic,y-vtic);
				(*t->vector)(x+htic,y-vtic);
				(*t->vector)(x+htic,y+vtic);
				(*t->vector)(x-htic,y+vtic);
				(*t->vector)(x-htic,y-vtic);
				(*t->move)(x,y);
				(*t->vector)(x,y);
				break;
		case 3: /* do X */ 
				(*t->move)(x-htic,y-vtic);
				(*t->vector)(x-htic,y-vtic);
				(*t->vector)(x+htic,y+vtic);
				(*t->move)(x-htic,y+vtic);
				(*t->vector)(x-htic,y+vtic);
				(*t->vector)(x+htic,y-vtic);
				break;
		case 4: /* do triangle */ 
				(*t->move)(x,y+(4*vtic/3));
				(*t->vector)(x-(4*htic/3),y-(2*vtic/3));
				(*t->vector)(x+(4*htic/3),y-(2*vtic/3));
				(*t->vector)(x,y+(4*vtic/3));
				(*t->move)(x,y);
				(*t->vector)(x,y);
				break;
		case 5: /* do star */ 
				(*t->move)(x-htic,y);
				(*t->vector)(x-htic,y);
				(*t->vector)(x+htic,y);
				(*t->move)(x,y-vtic);
				(*t->vector)(x,y-vtic);
				(*t->vector)(x,y+vtic);
				(*t->move)(x-htic,y-vtic);
				(*t->vector)(x-htic,y-vtic);
				(*t->vector)(x+htic,y+vtic);
				(*t->move)(x-htic,y+vtic);
				(*t->vector)(x-htic,y+vtic);
				(*t->vector)(x+htic,y-vtic);
				break;
	}
}


/*
 * general point routine
 */
line_and_point(x,y,number)
int x,y,number;
{
	/* temporary(?) kludge to allow terminals with bad linetypes 
		to make nice marks */

	(*term_tbl[term].linetype)(NICE_LINE);
	do_point(x,y,number);
}

/* 
 * general arrow routine
 */
#define ROOT2 (1.41421)		/* sqrt of 2 */

do_arrow(sx, sy, ex, ey)
	int sx,sy;			/* start point */
	int ex, ey;			/* end point (point of arrowhead) */
{
    register struct termentry *t = &term_tbl[term];
    int len = (t->h_tic + t->v_tic)/2; /* arrowhead size = avg of tic sizes */
    extern double sqrt();

    /* draw the line for the arrow. That's easy. */
    (*t->move)(sx, sy);
    (*t->vector)(ex, ey);

    /* now draw the arrow head. */
    /* we put the arrowhead marks at 45 degrees to line */
    if (sx == ex) {
	   /* vertical line, special case */
	   int delta = ((float)len / ROOT2 + 0.5);
	   if (sy < ey)
		delta = -delta;	/* up arrow goes the other way */
	   (*t->move)(ex - delta, ey + delta);
	   (*t->vector)(ex,ey);
	   (*t->vector)(ex + delta, ey + delta);
    } else {
	   int dx = sx - ex;
	   int dy = sy - ey;
	   double coeff = len / sqrt(2.0*((double)dx*(double)dx 
				+ (double)dy*(double)dy));
	   int x,y;			/* one endpoint */

	   x = (int)( ex + (dx + dy) * coeff );
	   y = (int)( ey + (dy - dx) * coeff );
	   (*t->move)(x,y);
	   (*t->vector)(ex,ey);

	   x = (int)( ex + (dx - dy) * coeff );
	   y = (int)( ey + (dy + dx) * coeff );
	   (*t->vector)(x,y);
    }

}


#include "x11.trm"

/* Dummy functions for unavailable features */

/* change angle of text.  0 is horizontal left to right.
* 1 is vertical bottom to top (90 deg rotate)  
*/
int null_text_angle()
{
return FALSE ;	/* can't be done */
}

/* change justification of text.  
 * modes are LEFT (flush left), CENTRE (centred), RIGHT (flush right)
 */
int null_justify_text()
{
return FALSE ;	/* can't be done */
}


/* Change scale of plot.
 * Parameters are x,y scaling factors for this plot.
 * Some terminals (eg latex) need to do scaling themselves.
 */
int null_scale()
{
return FALSE ;	/* can't be done */
}


UNKNOWN_null()
{
}


/*
 * term_tbl[] contains an entry for each terminal.  "unknown" must be the
 *   first, since term is initialized to 0.
 */
struct termentry term_tbl[] = {
    {"unknown", "Unknown terminal type - not a plotting device",
	  100, 100, 1, 1,
	  1, 1, UNKNOWN_null, UNKNOWN_null, 
	  UNKNOWN_null, null_scale, UNKNOWN_null, UNKNOWN_null, UNKNOWN_null, 
	  UNKNOWN_null, UNKNOWN_null, null_text_angle, 
	  null_justify_text, UNKNOWN_null, UNKNOWN_null, UNKNOWN_null}
    ,{"x11", "X11 Window System",
	   X11_XMAX, X11_YMAX, X11_VCHAR, X11_HCHAR, 
	   X11_VTIC, X11_HTIC, X11_init, X11_reset, 
	   X11_text, null_scale, X11_graphics, X11_move, X11_vector, 
	   X11_linetype, X11_put_text, null_text_angle, 
	   X11_justify_text, line_and_point, do_arrow, UNKNOWN_null}
    ,{"X11", "X11 Window System - multi-color points",
	   X11_XMAX, X11_YMAX, X11_VCHAR, X11_HCHAR, 
	   X11_VTIC, X11_HTIC, X11_init, X11_reset, 
	   X11_text, null_scale, X11_graphics, X11_move, X11_vector, 
	   X11_linetype, X11_put_text, null_text_angle, 
	   X11_justify_text, do_point, do_arrow, UNKNOWN_null}
};

#define TERMCOUNT (sizeof(term_tbl)/sizeof(struct termentry))


/* change_term: get terminal number from name and set terminal type */
/* returns -1 if unknown, -2 if ambiguous, >=0 is terminal number */
int
change_term(name, length)
	char *name;
	int length;
{
    int i, t = -1;

    for (i = 0; i < TERMCOUNT; i++) {
	   if (!strncmp(name,term_tbl[i].name,length)) {
		  if (t != -1)
		    return(-2);	/* ambiguous */
		  t = i;
	   }
    }

    if (t == -1)			/* unknown */
	 return(t);

    /* Success: set terminal type now */

    term = t;
    term_init = FALSE;
    name = term_tbl[term].name;

    return(t);
}
