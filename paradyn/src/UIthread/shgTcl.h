// shgTcl.h
// Ariel Tamches

// Implementations of new commands and tk bindings related to the new shg ui.

/* $Log: shgTcl.h,v $
/* Revision 1.2  1996/02/02 18:51:36  tamches
/* added shgDrawKeyCallback and shgDrawTipsCallback
/*
 * Revision 1.1  1995/10/17 22:09:06  tamches
 * initial version, for the new search history graph.
 *
 */

#ifndef _SHG_TCL_H_
#define _SHG_TCL_H_

#include "tcl.h"
#include "tk.h"

void installShgCommands(Tcl_Interp *);
void unInstallShgCommands(Tcl_Interp *);

void initiateShgRedraw(Tcl_Interp *, bool doubleBuffer);

#ifdef PARADYN
// the shg test program does not need this here
void shgDrawKeyCallback(bool newValue);
void shgDrawTipsCallback(bool newValue);
#endif

#endif
