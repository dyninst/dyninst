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

/* $Id: dg2.C,v 1.15 2003/07/18 15:45:38 schendel Exp $ */

#include <iostream>

#include "common/h/headers.h"

#include "tcl.h"
#include "tk.h"

#include "pdutil/h/pdsocket.h"
#include "pdutil/h/pddesc.h"
#include "common/h/Types.h"
#include "visi/h/visualization.h"

#include "dg2.h"

#include "tableVisiTcl.h"
#include "pdutil/h/TclTools.h"

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
		       TCLCONST char *argv[]) {

  std::ostringstream resstr;

  if (argc == 0) {
     resstr << "USAGE: Dg <option> [args...]\n" << std::ends;
     SetInterpResult(interp, resstr);
     return CMDERROR;
  }

  for (cmdTabEntry *C = Dg_Cmds; C->cmdname!=NULL; C++) {
     if (strcmp(argv[0], C->cmdname) == 0) {
        if (argc-1 == C->numargs) 
   	   return C->index; // successful parsing

        resstr << argv[0] << ": wrong number of args ("
            << argc-1 << "). Should be "
            << C->numargs << "\n" << std::ends;
        SetInterpResult(interp, resstr);
        return CMDERROR;
     }
  }

  resstr << "unknown option (" << argv[0] << ")\n" << std::ends;
  SetInterpResult(interp, resstr);
  return CMDERROR;
}

int Dg_TclCommand(ClientData, Tcl_Interp *interp, 
		  int argc, TCLCONST char *argv[]) {
  // entrypoint to the tcl "Dg" command we've installed
  // all the sprintf()'s are rather slow...

  // parse the arguments, using global vrble Dg_Cmds[] to tell what's what.
  int cmdDex = findCommand(interp, argc-1, argv+1);
  if (cmdDex == CMDERROR)
    return TCL_ERROR;

  int m, r, buck; // metric number, resource number, bucket number
  std::ostringstream resstr;
  int ret = TCL_OK;


  switch(cmdDex) {
  case AGGREGATE:   
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    resstr << visi_AverageValue(m,r) << std::ends;
    break;

  case BINWIDTH:     
    resstr << visi_BucketWidth() << std::ends;
    break;

  case FIRSTBUCKET:
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    resstr << visi_FirstValidBucket(m,r) << std::ends;
    break;

  case LASTBUCKET:
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    resstr << visi_LastBucketFilled(m,r) << std::ends;
    break;

  case METRICNAME:  
    m = atoi(argv[2]);
    resstr << visi_MetricName(m) << std::ends;
    break;

  case METRICUNITS:  
    m = atoi(argv[2]);
    resstr << visi_MetricUnits(m) << std::ends;
    break;

  case NUMBINS:     
    resstr << visi_NumBuckets() << std::ends;
    break;

  case NUMMETRICS:  
    resstr << visi_NumMetrics() << std::ends;
    break;

  case NUMRESOURCES:
    resstr << visi_NumResources() << std::ends;
    break;

  case RESOURCENAME:
    r = atoi(argv[2]);
    resstr << visi_ResourceName(r) << std::ends;
    break;

  case STARTSTREAM:       
    visi_GetMetsRes(argv[2], atoi(argv[3]));
    break;

  case STOPSTREAM:
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    visi_StopMetRes(m, r);
    break;

  case DGSUM:         
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    resstr << visi_SumValue(m,r) << std::ends;
    break;

  case DGVALID:
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    resstr << visi_Valid(m,r) << std::ends;
    break;

  case DGENABLED:
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    resstr << visi_Enabled(m,r) << std::ends;
    break;

  case VALUE:       
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    buck = atoi(argv[4]);
    resstr << visi_DataValue(m,r,buck) << std::ends;
    break;

  case MYPHASENAME:
    resstr << visi_GetMyPhaseName() << std::ends;
    break;

  default:
    resstr << "Internal error (func findCommand)\n" << std::ends;
    ret = TCL_ERROR;
    break;
  }

  SetInterpResult(interp, resstr);

  return ret;
}

int Dg2_Init(Tcl_Interp *interp) {
   Tcl_CreateCommand(interp, "Dg", Dg_TclCommand, 
		    (ClientData *) NULL,(Tcl_CmdDeleteProc *) NULL);
 
   return TCL_OK;
}
