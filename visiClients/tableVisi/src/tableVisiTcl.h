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

// tableVisiTcl.h
// Ariel Tamches

/*
 * $Log: tableVisiTcl.h,v $
 * Revision 1.6  2004/03/23 01:12:49  eli
 * Updated copyright string
 *
 * Revision 1.5  1999/07/13 17:16:13  pcroth
 * Fixed ordering problem of destroying GUI and destructing static variable
 * pdLogo::all_logos.  On NT, the static variable is destroyed before the
 * GUI, but a callback for the GUI ends up referencing the variable causing
 * an access violation error.
 *
 * Revision 1.4  1996/08/16 21:37:02  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.3  1996/04/30 20:17:55  tamches
 * added Dg2PhaseDataCallback
 *
 * Revision 1.2  1995/11/08 21:54:35  tamches
 * interface now contains just 2 callbacks: newdata and addmetrics
 *
 * Revision 1.1  1995/11/04 00:48:24  tamches
 * First version of new table visi
 *
 */

#ifndef _TABLE_VISI_TCL_H_
#define _TABLE_VISI_TCL_H_

#include "tkTools.h"

int Dg2NewDataCallback(int lastBucket);
int Dg2AddMetricsCallback(int);
//int Dg2Fold(int);
//int Dg2InvalidMetricsOrResources(int);
//int Dg2PhaseNameCallback(int);
int Dg2PhaseDataCallback(int);
int Dg2ParadynExitedCallback(int);

void installTableVisiCommands(Tcl_Interp *);
void unInstallTableVisiCommands(Tcl_Interp *);

#endif
