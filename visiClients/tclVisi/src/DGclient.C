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

/*
 * $Id: DGclient.C,v 1.27 2004/03/23 01:12:50 eli Exp $
 * DGclient.C -- Code for the visi<->tcl interface.
 */

#include <iostream>

#include "common/h/headers.h"
#include "common/h/std_namesp.h"
#include "tcl.h"
#include "tk.h"

#include "pdutil/h/pdsocket.h"
#include "pdutil/h/pddesc.h"
#include "common/h/Types.h"
#include "visi/h/visualization.h"
#include "pdutil/h/TclTools.h"




extern Tcl_Interp *MainInterp;

void my_visi_callback(void* , int* , long unsigned int* ) {
    if (-1 == visi_callback())
       exit(1);
}

int Dg_Add(int) {
   // Gets called by visi lib when it detects new METRICS and/or RESOURCES

   const int retval = Tcl_Eval(MainInterp, "DgConfigCallback");
   if (retval == TCL_ERROR)
      cerr << Tcl_GetStringResult(MainInterp) << endl;

   return retval;
}

int Dg_Data(int lastBucket) {
   // New data has arrived.
   // We are passed the bucket number.  We can grab data for all current
   // metric/resource pairs and do something.

   // Here, we just invoke the tcl script "DgDataCallback", passing lastBucket

   char buffer[100];
   sprintf(buffer, "DgDataCallback %d", lastBucket);
   const int retval = Tcl_Eval(MainInterp, buffer);
   if (retval == TCL_ERROR)
      cerr << Tcl_GetStringResult(MainInterp) << endl;

   return retval;
}

int Dg_Fold(int) {
   const int retval=Tcl_Eval(MainInterp, "DgFoldCallback");
   if (retval == TCL_ERROR)
      cerr << Tcl_GetStringResult(MainInterp) << endl;

   return retval;
}

int Dg_Invalid(int) {
   const int retval=Tcl_Eval(MainInterp, "DgInvalidCallback");
   if (retval == TCL_ERROR)
      cerr << Tcl_GetStringResult(MainInterp) << endl;

   return retval;
}

int Dg_PhaseStart(int which) {
   char buffer[100];
   sprintf(buffer, "DgPhaseStartCallback %d", which);
   const int retval = Tcl_Eval(MainInterp, buffer);
   if (retval == TCL_ERROR)
      cerr << Tcl_GetStringResult(MainInterp) << endl;

  return retval;
}

int Dg_PhaseEnd(int which) {
   char buffer[100];
   sprintf(buffer, "DgPhaseEndCallback %d", which);
   const int retval = Tcl_Eval(MainInterp, buffer);
   if (retval == TCL_ERROR)
      cerr << Tcl_GetStringResult(MainInterp) << endl;

  return retval;
}

int Dg_PhaseData(int) {
   const int retval=Tcl_Eval(MainInterp, "DgPhaseDataCallback");
   if (retval == TCL_ERROR)
      cerr << Tcl_GetStringResult(MainInterp) << endl;

  return retval;
}

int Dg_Exited(int) {
   const int retval=Tcl_Eval(MainInterp, "DgParadynExitedCallback");
   if (retval == TCL_ERROR){
      // cerr << Tcl_GetStringResult(MainInterp) << endl;
      exit(-1);
   }

  return retval;
}

#define   AGGREGATE        0
#define   BINWIDTH         1
#define   METRICNAME       2
#define   METRICUNITS      3
#define   NUMBINS          4
#define   NUMMETRICS       5
#define   NUMRESOURCES     6
#define   DEFINEPHASE      7
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
#define   NUMPHASES        18
#define   PHASENAME        19
#define   PHASESTARTTIME   20
#define   PHASEENDTIME     21
#define   MYPHASENAME      22
#define   MYPHASESTARTTIME 23
#define   MYPHASEHANDLE    24
#define   METRICAVELAB     25
#define   METRICSUMLAB	   26

struct cmdTabEntry {
   char *cmdname;
   int index;
   int numargs;
};

static struct cmdTabEntry Dg_Cmds[] = {
  {"average",      AGGREGATE,       2},
  {"binwidth",     BINWIDTH,        0},
  {"firstbucket",  FIRSTBUCKET,     2},
  {"lastbucket",   LASTBUCKET,      2},
  {"metricname",   METRICNAME,      1},
  {"metriclabel",  METRICUNITS,     1},
  {"numbins",      NUMBINS,         0},
  {"nummetrics",   NUMMETRICS,      0},
  {"numresources", NUMRESOURCES,    0},
  {"phase",        DEFINEPHASE,     3},
  {"resourcename", RESOURCENAME,    1},
  {"start",        STARTSTREAM,     2},
  {"stop",         STOPSTREAM,      2},
  {"sum",          DGSUM,           2},
  {"valid",        DGVALID,         2},
  {"enabled",      DGENABLED,       2},
  {"value",        VALUE,           3},
  {"phasename",    PHASENAME,       1},
  {"phasestartT",  PHASESTARTTIME,  1},
  {"phaseendT",    PHASEENDTIME,    1},
  {"myphasename",  MYPHASENAME,     0},
  {"myphasestartT", MYPHASESTARTTIME, 0},
  {"myphasehandle", MYPHASEHANDLE,  0},
  {"numphases",     NUMPHASES,      0},
  {"metricavelabel",  METRICAVELAB, 1},
  {"metricsumlabel",  METRICSUMLAB, 1},
  {NULL,           CMDERROR,        0}
};

int findCommand(Tcl_Interp *interp, 
		int argc, TCLCONST char *argv[]) {

  std::ostringstream resstr;

  if (argc == 0) {
    resstr << "USAGE: Dg <option> [args...]\n" << std::ends;
    SetInterpResult(interp, resstr);
    return CMDERROR;
  }
  for (cmdTabEntry *C = Dg_Cmds; C->cmdname; C++) {
    if (strcmp(argv[0], C->cmdname) == 0) {
      if ((argc-1) == C->numargs) 
	return C->index; // successful parsing

      resstr << argv[0] << ": wrong number of args ("
          << argc-1 << "). Should be "
          << C->numargs << '\n' << std::ends;
      SetInterpResult(interp, resstr);
      return CMDERROR;
    }
  }

  resstr << "unknown option (" << argv[0] << ")\n" << std::ends;
  SetInterpResult(interp, resstr);
  return CMDERROR;
}

int Dg_TclCommand(ClientData,
		  Tcl_Interp *interp, 
		  int argc, 
		  TCLCONST char *argv[]) {
  const int cmdDex = findCommand(interp, argc-1, argv+1);
  if (cmdDex == CMDERROR)
     return TCL_ERROR;

  int m, r, buck;
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
    resstr << visi_MetricLabel(m) << std::ends;
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

  case DEFINEPHASE: {
    // argv[2] --> phase name (currently unused!)
    // argv[3] --> with perf consult (int value)
    // argv[4] --> with visis (int value)

    int withPerfConsult = 0;
    int withVisis = 0;
    withPerfConsult = atoi(argv[3]);
    withVisis = atoi(argv[4]);


    visi_DefinePhase(NULL, (bool)withPerfConsult, (bool)withVisis);
       // let paradyn pick the phase's name
    break;
  }

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

  case NUMPHASES:
    resstr << visi_NumPhases() << std::ends;
    break;

  case PHASENAME:
    m = atoi(argv[2]);
    resstr << visi_GetPhaseName(m) << std::ends;
    break;

  case PHASESTARTTIME:
    m = atoi(argv[2]);
    resstr << visi_GetPhaseStartTime(m) << std::ends;
    break;

  case PHASEENDTIME:
    m = atoi(argv[2]);
    resstr << visi_GetPhaseEndTime(m) << std::ends;
    break;

  case MYPHASENAME:
    resstr << visi_GetMyPhaseName() << std::ends;
    break;

  case MYPHASESTARTTIME:
    resstr << visi_GetStartTime() << std::ends;
    break;

  case MYPHASEHANDLE:
    resstr << visi_GetMyPhaseHandle() << std::ends;
    break;

  case METRICAVELAB:
    m = atoi(argv[2]);
    resstr << visi_MetricAveLabel(m) << std::ends;
    break;

  case METRICSUMLAB:
    m = atoi(argv[2]);
    resstr << visi_MetricSumLabel(m) << std::ends;
    break;

  default:
    resstr << "Internal error (func findCommand)\n" << std::ends;
    ret = TCL_ERROR;
    break;
  }

  // now set the interpreter's result
  SetInterpResult(interp, resstr);

  return ret;
}

int Dg_Init(Tcl_Interp *interp) {
   PDSOCKET visi_sock=visi_Init();
   if (visi_sock == INVALID_PDSOCKET) {
      cerr << "tclVisi: could not initialize visilib" << endl;
      exit(-1);
   }

  (void) visi_RegistrationCallback(ADDMETRICSRESOURCES,Dg_Add); 
  (void) visi_RegistrationCallback(DATAVALUES,Dg_Data); 
  (void) visi_RegistrationCallback(FOLD,Dg_Fold); 
  (void) visi_RegistrationCallback(INVALIDMETRICSRESOURCES,Dg_Invalid);
  (void) visi_RegistrationCallback(PHASESTART,Dg_PhaseStart);
  (void) visi_RegistrationCallback(PHASEEND,Dg_PhaseEnd);
  (void) visi_RegistrationCallback(PHASEDATA,Dg_PhaseData);
  (void) visi_RegistrationCallback(PARADYNEXITED,Dg_Exited);

  Tcl_CreateCommand(interp, "Dg", Dg_TclCommand, 
		    (ClientData *) NULL,(Tcl_CmdDeleteProc *) NULL);
 
	// Arrange for my_visi_callback() to be called whenever data is waiting
	// to be read off of descriptor "visi_sock".
	Tcl_Channel visi_chan = 
	  Tcl_MakeTcpClientChannel((ClientData)(Address)(PDDESC)visi_sock);
	Tcl_CreateChannelHandler(visi_chan,
				 TCL_READABLE,
				 (Tcl_FileProc*)my_visi_callback,
				 0);

  return TCL_OK;
}
