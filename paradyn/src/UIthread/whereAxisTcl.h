// whereAxisTcl.h
// Ariel Tamches

// Implementations of new commands and tk bindings related to the where axis.

/* $Log: whereAxisTcl.h,v $
/* Revision 1.1  1995/07/17 04:59:11  tamches
/* First version of the new where axis
/*
 */

#ifndef _WHERE_AXIS_TCL_H_
#define _WHERE_AXIS_TCL_H_

#include <tclclean.h>
#include <tkclean.h>

void installWhereAxisCommands(Tcl_Interp *);
void unInstallWhereAxisCommands(Tcl_Interp *);

void initiateWhereAxisRedraw(Tcl_Interp *, bool doubleBuffer);

#endif
