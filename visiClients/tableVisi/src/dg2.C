/*
 * Copyright (c) 1996-1999 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

// dg2.C
// implementation of the "Dg" tcl command

/*
 * $Log: dg2.C,v $
 * Revision 1.8  1999/03/13 15:24:04  pcroth
 * Added support for building under Windows NT
 *
 * Revision 1.7  1996/08/16 21:36:52  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.6  1996/04/30 20:16:14  tamches
 * added MYPHASENAME
 *
 * Revision 1.5  1996/02/23 17:49:44  tamches
 * removed DEFINEPHASE
 *
 * Revision 1.4  1996/01/19 20:56:18  newhall
 * changes due to visiLib interface changes
 *
 * Revision 1.3  1996/01/17 18:31:36  newhall
 * changes due to new visiLib
 *
 * Revision 1.2  1995/11/08  21:45:56  tamches
 * specialized s.t. only the implementation of the "Dg" tcl command is here
 *
 * Revision 1.1  1995/11/04 00:44:11  tamches
 * First version of new table visi
 *
 */

#include <stdlib.h> // exit()
#include <iostream.h>


#include "util/h/headers.h"
#include "util/h/pdsocket.h"
#include "visi/h/visualization.h"

#include "tcl.h"
#include "tk.h"
#include "tkTools.h"

#include "dg2.h"

#include "tableVisiTcl.h"

#define   AGGREGATE        0
#define   BINWIDTH         1
#define   FOLDMETHOD       2
#define   METRICNAME       3
#define   METRICUNITS      4
#define   NUMBINS          5
#define   NUMMETRICS       6
#define   NUMRESOURCES     7
#define   RESOURCENAME     8
#define   STARTSTREAM      9
#define   STOPSTREAM       10
#define   DGSUM            11
#define   DGVALID          12
#define   DGENABLED        13
#define   VALUE            14
#define   CMDERROR         15
#define   LASTBUCKET       16
#define   FIRSTBUCKET      17
#define   MYPHASENAME      18

struct cmdTabEntry {
   const char *cmdname;
   int index;
   int numargs;
};

static struct cmdTabEntry Dg_Cmds[] = {
  {"aggregate",    AGGREGATE,       2},
  {"binwidth",     BINWIDTH,        0},
  {"firstbucket",  FIRSTBUCKET,     2},
  {"foldmethod",   FOLDMETHOD,      2},
  {"lastbucket",   LASTBUCKET,      2},
  {"metricname",   METRICNAME,      1},
  {"metricunits",  METRICUNITS,     1},
  {"numbins",      NUMBINS,         0},
  {"nummetrics",   NUMMETRICS,      0},
  {"numresources", NUMRESOURCES,    0},
  {"resourcename", RESOURCENAME,    1},
  {"start",        STARTSTREAM,     2},
  {"stop",         STOPSTREAM,      2},
  {"sum",          DGSUM,           2},
  {"valid",        DGVALID,         2},
  {"enabled",      DGENABLED,       2},
  {"value",        VALUE,           3},
  {"myphasename",  MYPHASENAME,     0},
  {NULL,           CMDERROR,        0}
};

int findCommand(Tcl_Interp *interp, 
		       int argc, 
		       char *argv[]) {

  if (argc == 0) {
     sprintf(interp->result, "USAGE: Dg <option> [args...]\n");
     return CMDERROR;
  }

  for (cmdTabEntry *C = Dg_Cmds; C->cmdname!=NULL; C++) {
     if (strcmp(argv[0], C->cmdname) == 0) {
        if (argc-1 == C->numargs) 
   	   return C->index; // successful parsing

        sprintf(interp->result, 
	      "%s: wrong number of args (%d). Should be %d\n",
	      argv[0], argc-1, C->numargs);
        return CMDERROR;
     }
  }

  sprintf(interp->result, "unknown option (%s)\n", argv[0]);
  return CMDERROR;
}

int Dg_TclCommand(ClientData, Tcl_Interp *interp, 
		  int argc, char *argv[]) {
  // entrypoint to the tcl "Dg" command we've installed
  // all the sprintf()'s are rather slow...

  // parse the arguments, using global vrble Dg_Cmds[] to tell what's what.
  int cmdDex = findCommand(interp, argc-1, argv+1);
  if (cmdDex == CMDERROR)
    return TCL_ERROR;

  int m, r, buck; // metric number, resource number, bucket number

  switch(cmdDex) {
  case AGGREGATE:   
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    sprintf(interp->result,"%g", visi_AverageValue(m,r));
    return TCL_OK;

  case BINWIDTH:     
    sprintf(interp->result, "%g", visi_BucketWidth());
    return TCL_OK;

  case FIRSTBUCKET:
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    sprintf(interp->result,"%d", visi_FirstValidBucket(m,r)); 
    return TCL_OK;

  case LASTBUCKET:
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    sprintf(interp->result,"%d", visi_LastBucketFilled(m,r));
    return TCL_OK;

  case METRICNAME:  
    m = atoi(argv[2]);
    sprintf(interp->result, "%s", visi_MetricName(m));
    return TCL_OK;

  case METRICUNITS:  
    m = atoi(argv[2]);
    sprintf(interp->result, "%s", visi_MetricUnits(m));
    return TCL_OK;

  case NUMBINS:     
    sprintf(interp->result, "%d", visi_NumBuckets());
    return TCL_OK;

  case NUMMETRICS:  
    sprintf(interp->result, "%d", visi_NumMetrics());
    return TCL_OK;

  case NUMRESOURCES:
    sprintf(interp->result, "%d", visi_NumResources());
    return TCL_OK;

  case RESOURCENAME:
    r = atoi(argv[2]);
    sprintf(interp->result, "%s", visi_ResourceName(r));
    return TCL_OK;

  case STARTSTREAM:       
    visi_GetMetsRes(argv[2], atoi(argv[3]));
    return TCL_OK;

  case STOPSTREAM:
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    visi_StopMetRes(m, r);
    return TCL_OK;

  case DGSUM:         
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    sprintf(interp->result,"%g", visi_SumValue(m,r));
    return TCL_OK;

  case DGVALID:
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    sprintf(interp->result, "%d", visi_Valid(m,r));
    return TCL_OK;

  case DGENABLED:
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    sprintf(interp->result, "%d", visi_Enabled(m,r));
    return TCL_OK;

  case VALUE:       
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    buck = atoi(argv[4]);
    sprintf(interp->result,"%g", visi_DataValue(m,r,buck));
    return TCL_OK;

  case MYPHASENAME:
    sprintf(interp->result, "%s", visi_GetMyPhaseName());
    return TCL_OK;
  }

  sprintf(interp->result, "Internal error (func findCommand)\n");
  return TCL_ERROR;
}

int Dg2_Init(Tcl_Interp *interp) {
   Tcl_CreateCommand(interp, "Dg", Dg_TclCommand, 
		    (ClientData *) NULL,(Tcl_CmdDeleteProc *) NULL);
 
   return TCL_OK;
}
