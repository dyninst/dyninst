/*
 * Copyright (c) 1996 Barton P. Miller
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

/*
 *  DGclient.C -- Code for the visi<->tcl interface.
 *    
 * $Log: DGclient.C,v $
 * Revision 1.17  1996/08/16 21:37:39  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.16  1996/08/05 07:13:24  tamches
 * update for tcl 7.5
 *
 * Revision 1.15  1996/04/04 22:28:58  newhall
 * changed type of args to visi_DefinePhase to match visi interface
 *
 * Revision 1.14  1996/02/23  17:51:11  tamches
 * DEFINEPHASE now takes 3 params instead of 1
 *
 * Revision 1.13  1996/02/11 21:25:09  tamches
 * added param to start-phase
 *
 * Revision 1.12  1996/01/26 22:02:00  newhall
 * added myphasename, myphasestartT, myphasehandle
 *
 * Revision 1.11  1996/01/19  20:56:29  newhall
 * changes due to visiLib interface changes
 *
 */

#include <stdlib.h>
#include <iostream.h>
#include <tcl.h>
#include <tk.h>
#include "visi/h/visualization.h"

extern Tcl_Interp *MainInterp;

void my_visi_callback(void* , int* , long unsigned int* ) {
    if (-1 == visi_callback())
       exit(1);
}

int Dg_Add(int) {
   // Gets called by visi lib when it detects new METRICS and/or RESOURCES

   const int retval = Tcl_Eval(MainInterp, "DgConfigCallback");
   if (retval == TCL_ERROR)
      cerr << MainInterp->result << endl;

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
      cerr << MainInterp->result << endl;

   return retval;
}

int Dg_Fold(int) {
   const int retval=Tcl_Eval(MainInterp, "DgFoldCallback");
   if (retval == TCL_ERROR)
      cerr << MainInterp->result << endl;

   return retval;
}

int Dg_Invalid(int) {
   const int retval=Tcl_Eval(MainInterp, "DgInvalidCallback");
   if (retval == TCL_ERROR)
      cerr << MainInterp->result << endl;

   return retval;
}

int Dg_PhaseStart(int which) {
   char buffer[100];
   sprintf(buffer, "DgPhaseStartCallback %d", which);
   const int retval = Tcl_Eval(MainInterp, buffer);
   if (retval == TCL_ERROR)
      cerr << MainInterp->result << endl;

  return retval;
}

int Dg_PhaseEnd(int which) {
   char buffer[100];
   sprintf(buffer, "DgPhaseEndCallback %d", which);
   const int retval = Tcl_Eval(MainInterp, buffer);
   if (retval == TCL_ERROR)
      cerr << MainInterp->result << endl;

  return retval;
}

int Dg_PhaseData(int) {
   const int retval=Tcl_Eval(MainInterp, "DgPhaseDataCallback");
   if (retval == TCL_ERROR)
      cerr << MainInterp->result << endl;

  return retval;
}

int Dg_Exited(int) {
   const int retval=Tcl_Eval(MainInterp, "DgParadynExitedCallback");
   if (retval == TCL_ERROR){
      // cerr << MainInterp->result << endl;
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
		int argc, char *argv[]) {
  if (argc == 0) {
    sprintf(interp->result, "USAGE: Dg <option> [args...]\n");
    return CMDERROR;
  }
  for (cmdTabEntry *C = Dg_Cmds; C->cmdname; C++) {
    if (strcmp(argv[0], C->cmdname) == 0) {
      if ((argc-1) == C->numargs) 
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

int Dg_TclCommand(ClientData,
		  Tcl_Interp *interp, 
		  int argc, 
		  char *argv[]) {
  const int cmdDex = findCommand(interp, argc-1, argv+1);
  if (cmdDex == CMDERROR)
     return TCL_ERROR;

  int m, r, buck;

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
    sprintf(interp->result, "%s", visi_MetricLabel(m));
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
    return TCL_OK;
  }

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

  case NUMPHASES:
    sprintf(interp->result, "%d", visi_NumPhases());
    return TCL_OK;

  case PHASENAME:
    m = atoi(argv[2]);
    sprintf(interp->result, "%s", visi_GetPhaseName(m));
    return TCL_OK;

  case PHASESTARTTIME:
    m = atoi(argv[2]);
    sprintf(interp->result, "%f", visi_GetPhaseStartTime(m));
    return TCL_OK;

  case PHASEENDTIME:
    m = atoi(argv[2]);
    sprintf(interp->result, "%f", visi_GetPhaseEndTime(m));
    return TCL_OK;

  case MYPHASENAME:
    sprintf(interp->result, "%s", visi_GetMyPhaseName());
    return TCL_OK;

  case MYPHASESTARTTIME:
    sprintf(interp->result, "%f", visi_GetStartTime());
    return TCL_OK;

  case MYPHASEHANDLE:
    sprintf(interp->result, "%d", visi_GetMyPhaseHandle());
    return TCL_OK;



  case METRICAVELAB:
    m = atoi(argv[2]);
    sprintf(interp->result, "%s", visi_MetricAveLabel(m));
    return TCL_OK;

  case METRICSUMLAB:
    m = atoi(argv[2]);
    sprintf(interp->result, "%s", visi_MetricSumLabel(m));
    return TCL_OK;
   
  }

  sprintf(interp->result, "Internal error (func findCommand)\n");
  return TCL_ERROR;
}

int Dg_Init(Tcl_Interp *interp) {
   int fd=visi_Init();
   if (fd < 0) {
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
  // to be read off of descriptor "fd".
  
  // New to tcl 7.5: need to call Tcl_GetFile() before Tcl_CreateFileHandler()
  Tcl_File theFdFile = Tcl_GetFile((ClientData)fd, TCL_UNIX_FD);
  Tcl_CreateFileHandler(theFdFile, TK_READABLE, (Tk_FileProc *) my_visi_callback, 0);
     // note that since we don't return "theFdFile", the opportunity to properly
     // call Tcl_FreeFile() when done is lost, but that's no huge bug.

  return TCL_OK;
}
