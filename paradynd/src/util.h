/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

/*
 * util.h - support functions.
 *
 * $Log: util.h,v $
 * Revision 1.1  1994/01/27 20:31:49  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.1  1993/03/19  22:51:05  hollings
 * Initial revision
 *
 *
 */
#ifndef UTIL_H
#define UTIL_H

extern "C" void *xmalloc(int size);

extern "C" void *xcalloc(int size, int count);

extern "C" void *xrealloc(void *ptr, int size);

#endif UTIL_H
