// tableVisiTcl.h
// Ariel Tamches

/*
 * $Log: tableVisiTcl.h,v $
 * Revision 1.1  1995/11/04 00:48:24  tamches
 * First version of new table visi
 *
 */

#ifndef _TABLE_VISI_TCL_H_
#define _TABLE_VISI_TCL_H_

#include "tkTools.h"

int Dg2NewDataCallback(int lastBucket);
int Dg2AddMetricsCallback(int);
int Dg2Fold(int);
int Dg2InvalidMetricsOrResources(int);
int Dg2PhaseNameCallback(int);

extern tkInstallIdle tableDrawWhenIdle;

void installTableVisiCommands(Tcl_Interp *);
void unInstallTableVisiCommands(Tcl_Interp *);

#endif
