// shgTcl.h
// Ariel Tamches

// Implementations of new commands and tk bindings related to the new shg ui.

/* $Log: shgTcl.h,v $
/* Revision 1.1  1995/10/17 22:09:06  tamches
/* initial version, for the new search history graph.
/*
 */

#ifndef _SHG_TCL_H_
#define _SHG_TCL_H_

#include "tclclean.h"
#include "tkclean.h"

void installShgCommands(Tcl_Interp *);
void unInstallShgCommands(Tcl_Interp *);

void initiateShgRedraw(Tcl_Interp *, bool doubleBuffer);

#endif
