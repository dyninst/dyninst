/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 *
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */

/************************************************************************
 * kludges.h: the name says it all. stop compiler warnings.
************************************************************************/

extern int fflush(FILE *);
extern int fprintf(FILE *, const char *, ...);
extern int printf(const char *, ...);
extern int fread(char *ptr, int size, int nitems, FILE *stream);

extern int fclose(FILE *stream);
extern int fflush(FILE *stream);

extern caddr_t sbrk(int incr);
