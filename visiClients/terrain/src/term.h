/* GNUPLOT - term.h */
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
 **************************************
 * term.h: terminal support definitions
 *   Edit this file depending on the set of terminals you wish to support.
 * Comment out the terminal types that you don't want or don't have, and
 * uncomment those that you want included. Be aware that some terminal 
 * types (eg, SUN, UNIXPLOT) will require changes in the makefile 
 * LIBS definition. 
 */

/*
 * Adapted for terrain plot:
 *      Chi-Ting Lam
 *
 */

/*
 * term.h - header file of term.c
 *
 * $Log: term.h,v $
 * Revision 1.2  1997/05/21 19:14:59  tung
 * Revised.
 *
 * Revision 1.1  1997/05/12 20:15:43  naim
 * Adding "Terrain" visualization to paradyn (commited by naim, done by tung).
 *
 * Revision 1.2  1992/05/19  17:39:36  lam
 * *** empty log message ***
 *
 * Revision 1.1  1992/05/19  07:21:56  lam
 * Initial revision
 *
 *
 */

#ifndef TERM_H
#define TERM_H

#define X11         /* X11R4 window system */

int do_point(int x, int y, int number);
int line_and_point(int x, int y, int number);
int do_arrow(int sx, int sy, int ex, int ey);
int null_text_angle();
int null_justify_text();
int null_scale();
int UNKNOWN_null();
int change_term(char* name, int length);

#endif
