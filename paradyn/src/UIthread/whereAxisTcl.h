// whereAxisTcl.h
// Ariel Tamches

// Implementations of new commands and tk bindings related to the where axis.

/* $Log: whereAxisTcl.h,v $
/* Revision 1.3  1996/01/09 23:55:58  tamches
/* added whereAxisDrawTipsCallback
/*
 * Revision 1.2  1995/10/17 22:21:57  tamches
 * put initiateWhereAxisRedraw within #ifdef PARADYN
 *
 * Revision 1.1  1995/07/17 04:59:11  tamches
 * First version of the new where axis
 *
 */

#ifndef _WHERE_AXIS_TCL_H_
#define _WHERE_AXIS_TCL_H_

#include "tcl.h"

void installWhereAxisCommands(Tcl_Interp *);
void unInstallWhereAxisCommands(Tcl_Interp *);

#ifdef PARADYN
// the where axis test program does not need this here
void initiateWhereAxisRedraw(Tcl_Interp *, bool);
void whereAxisDrawTipsCallback(bool newValue);
#endif

#endif
