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
// customized (for barchart) version of DGclient.C in tclVisi directory

/* $Id: dg2.C,v 1.30 2003/06/20 02:23:07 pcroth Exp $ */

// An updated version of DGClient.C for barchart2.C
// Contains several **deletions** to remove blt_barchart influences

#include <iostream.h>

#include "common/h/headers.h"

#include "tcl.h"
#include "tk.h"

#include "pdutil/h/pdsocket.h"
#include "pdutil/h/pddesc.h"
#include "common/h/Types.h"
#include "visi/h/visualization.h"

#include "tkTools.h" // myTclEval()

#include "dg2.h"
#include "barChartTcl.h"
#include "barChart.h"
#include "barChartUtil.h"
#include "pdutil/h/TclTools.h"

void my_visi_callback(void*, int*, long unsigned int*) {

   if (visi_callback() == -1)
      exit(0);
}

#define   AGGREGATE        0
#define   BINWIDTH         1
#define   FOLDMETHOD       2
#define   METRICNAME       3
#define   METRICUNITS      4
#define   METRICAVEUNITS   5
#define   METRICSUMUNITS   6
#define   NUMBINS          7
#define   NUMMETRICS       8
#define   NUMRESOURCES     9
#define   RESOURCENAME     10
#define   STARTSTREAM      11
#define   STOPSTREAM       12
#define   DGSUM            13
#define   DGVALID          14
#define   DGENABLED        15
#define   VALUE            16
#define   CMDERROR         17
#define   LASTBUCKET       18
#define   FIRSTBUCKET      19

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
  {"metricaveunits", METRICAVEUNITS,1},
  {"metricsumunits", METRICSUMUNITS,1},
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
  {NULL,           CMDERROR,        0}
};

int findCommand(Tcl_Interp *interp, 
		       int argc, 
		       TCLCONST char *argv[]) {

  ostrstream resstr;

  if (argc == 0) {
     resstr << "USAGE: Dg <option> [args...]\n" << ends;
     SetInterpResult(interp, resstr);
     return CMDERROR;
  }

  for (cmdTabEntry *C = Dg_Cmds; C->cmdname!=NULL; C++) {
     if (strcmp(argv[0], C->cmdname) == 0) {
        if (argc-1 == C->numargs) 
   	   return C->index; // successful parsing

        resstr << argv[0] << ": wrong number of args ("
            << argc-1 << "). Should be "
            << C->numargs << "\n" << ends;
        SetInterpResult(interp, resstr);
        return CMDERROR;
     }
  }

  resstr << "unknown option (" << argv[0] << ")\n" << ends;
  SetInterpResult(interp, resstr);
  return CMDERROR;
}

int Dg_TclCommand(ClientData,
		  Tcl_Interp *interp, 
		  int argc, TCLCONST char *argv[]) {
  // entrypoint to the tcl "Dg" command we've installed
  // all the sprintf()'s are rather slow...

  // parse the arguments, using global vrble Dg_Cmds[] to tell what's what.
  int cmdDex = findCommand(interp, argc-1, argv+1);
  if (cmdDex == CMDERROR)
    return TCL_ERROR;

  int m, r, buck; // metric number, resource number, bucket number
  ostrstream resstr;
  int ret = TCL_OK;


  switch(cmdDex) {
  case AGGREGATE:   
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    resstr << visi_AverageValue(m,r) << ends;
    break;

  case BINWIDTH:     
    resstr << visi_BucketWidth() << ends;
    break;

  case FIRSTBUCKET:
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    resstr << visi_FirstValidBucket(m,r) << ends;
    break;

  case LASTBUCKET:
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    resstr << visi_LastBucketFilled(m,r) << ends;
    break;

  case METRICNAME:  
    m = atoi(argv[2]);
    resstr << visi_MetricName(m) << ends;
    break;

  case METRICUNITS:  
    m = atoi(argv[2]);
    resstr << visi_MetricLabel(m) << ends;
    break;

  case METRICAVEUNITS:  
    m = atoi(argv[2]);
    resstr << visi_MetricAveLabel(m) << ends;
    break;

  case METRICSUMUNITS:  
    m = atoi(argv[2]);
    resstr << visi_MetricSumLabel(m) << ends;
    break;

  case NUMBINS:     
    resstr << visi_NumBuckets() << ends;
    break;

  case NUMMETRICS:  
    resstr << visi_NumMetrics() << ends;
    break;

  case NUMRESOURCES:
    resstr << visi_NumResources() << ends;
    break;

  case RESOURCENAME:
    r = atoi(argv[2]);
    resstr << visi_ResourceName(r) << ends;
    break;

  case STARTSTREAM:       
    visi_GetMetsRes(argv[2],0); 
    break;

  case STOPSTREAM:
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    visi_StopMetRes(m, r);
    break;

  case DGSUM:         
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    resstr << visi_SumValue(m,r) << ends;
    break;

  case DGVALID:
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    resstr << visi_Valid(m,r) << ends;
    break;

  case DGENABLED:
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    resstr << visi_Enabled(m,r) << ends;
    break;

  case VALUE:       
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    buck = atoi(argv[4]);
    resstr << visi_DataValue(m,r,buck) << ends;
    break;

  default:
    resstr << "Internal error (func findCommand)\n" << ends;
    break;
  }

  SetInterpResult(interp, resstr);
  return ret;
}

void (*UsersNewDataCallbackRoutine)(int firstBucket, int lastBucket);
   // we will call this routine for you when we get a new-data callback
   // from the visi lib (first, we do a bit of processing for you, such
   // as determining what the range is buckets you haven't seen yet is).

int Dg2_Init(Tcl_Interp *interp) {
   // initialize with the visi lib
   PDSOCKET visi_sock = visi_Init();
   if (visi_sock == PDSOCKET_ERROR) {
      cerr << "Dg2_Init() -- could not initialize with the visi lib" << endl;
      exit(5);
   }

   // Register C++ Callback routines with the visi lib when
   // certain events happen.  The most important (performance-wise)
   // is the DATAVALUES callback, which signals the arrival of
   // new barchart data.  We must process this callback very quickly,
   // in order to perturb the system as little as possible.

   if (visi_RegistrationCallback(ADDMETRICSRESOURCES,Dg2AddMetricsCallback)!=0)
      Tcl_Panic("Dg2_Init() -- couldn't install ADDMETRICSRESOURCES callback", NULL);

   if (visi_RegistrationCallback(FOLD, Dg2Fold) != 0)
      Tcl_Panic("Dg2_Init() -- couldn't install FOLD callback", NULL);

   if (visi_RegistrationCallback(INVALIDMETRICSRESOURCES, Dg2InvalidMetricsOrResources) != 0)
      Tcl_Panic("Dg2_Init() -- couldn't install INVALID callback", NULL);

   if (visi_RegistrationCallback(PHASESTART, Dg2PhaseNameCallback) != 0)
      Tcl_Panic("Dg2_Init() -- couldn't install PHASENAME callback", NULL);

   if (visi_RegistrationCallback(DATAVALUES, Dg2NewDataCallback) != 0)
      Tcl_Panic("Dg2_Init() -- couldn't install DATAVALUES callback", NULL);

   if (visi_RegistrationCallback(PARADYNEXITED, Dg2ParadynExitedCallback) != 0)
       Tcl_Panic("Dg2_Init() -- couldn't install PARADYNEXITED callback", NULL);

   // install "Dg" as a new tcl command; Dg_TclCommand() will be invoked when
   // a tcl script calls Dg
   Tcl_CreateCommand(interp, "Dg", Dg_TclCommand, 
		    (ClientData *) NULL,(Tcl_CmdDeleteProc *) NULL);
 
   // Arrange for my_visi_callback() to be called whenever data is waiting
   // to be read from visi socket
   Tcl_Channel visi_chan = 
     Tcl_MakeTcpClientChannel((ClientData)(Address)(PDDESC)visi_sock);
   Tcl_CreateChannelHandler(visi_chan,
			    TCL_READABLE,
			    (Tcl_FileProc*)my_visi_callback,
			    0);

   return TCL_OK;
}
