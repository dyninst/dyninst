// dg2.C
// customized (for barchart) version of DGclient.C in tclVisi directory

/* $Log: dg2.C,v $
/* Revision 1.16  1996/02/23 17:48:06  tamches
/* removed DEFINEPHASE
/*
 * Revision 1.15  1996/01/19 20:56:06  newhall
 * changes due to visiLib interface changes
 *
 * Revision 1.14  1996/01/17 18:31:14  newhall
 * changes due to new visiLib
 *
 * Revision 1.13  1996/01/10 21:11:15  tamches
 * added METRICAVEUNITS, METRICSUMUNITS
 *
 * Revision 1.12  1995/11/29 00:40:07  tamches
 * removed myTclEval
 *
 * Revision 1.11  1995/11/17 17:39:32  newhall
 * changed Dg start command, and call to GetMetsRes
 *
 * Revision 1.10  1995/11/17  17:32:27  newhall
 * changed Dg start command to take no arguments, replaced call to MetricUnits
 * with call to MetricLabel
 *
 * Revision 1.9  1995/09/22  19:23:41  tamches
 * removed warnings under g++ 2.7.0
 *
 * Revision 1.8  1995/08/06  22:11:48  tamches
 * removed some warnings by using myTclEval
 *
 * Revision 1.7  1995/02/26  02:01:48  newhall
 * added callback functions for new visiLib phase info.
 *
 * Revision 1.6  1994/11/06  10:24:04  tamches
 * minor cleanups (especially commenting)
 *
 * Revision 1.5  1994/10/11  21:59:47  tamches
 * Removed extra StartVisi() bug.
 * Implemented dataGrid[][].Enabled()
 *
 * Revision 1.4  1994/10/10  23:08:47  tamches
 * preliminary changes on the way to swapping the x and y axes
 *
 * Revision 1.3  1994/10/10  14:36:18  tamches
 * fixed some resizing bugs
 *
 * Revision 1.2  1994/09/29  20:05:39  tamches
 * minor cvs fixes
 *
 * Revision 1.1  1994/09/29  19:52:25  tamches
 * initial implementation.
 * This is a modified version of DGclient.C (tclVisi/src), specially
 * tuned for the barchart program.
 *
*/

// An updated version of DGClient.C for barchart2.C
// Contains several **deletions** to remove blt_barchart influences

#include <stdlib.h> // exit()
#include <iostream.h>

#include "tcl.h"
#include "tk.h"
#include "tkTools.h" // myTclEval()

#include "dg2.h"
#include "visi/h/visualization.h"
#include "barChartTcl.h"
#include "barChart.h"

void my_visi_callback(void*, int*, long unsigned int*) {
   if (visi_callback() == -1)
      exit(0);
}

int Dg2AddMetricsCallback(int) {
   myTclEval(MainInterp, "DgConfigCallback");

   // if necessary, the tcl program will call xAxisHasChanged and/or
   // yAxisHasChanged, which are commands we implement in barChart.C.
   // We take action then.

   return TCL_OK;
}

int Dg2Fold(int) {
   myTclEval(MainInterp, "DgFoldCallback");
   return TCL_OK;
}

int Dg2InvalidMetricsOrResources(int) {
   myTclEval(MainInterp, "DgInvalidCallback");
   return TCL_OK;
}

int Dg2PhaseNameCallback(int) {
   myTclEval(MainInterp, "DgPhaseCallback");
   return TCL_OK;
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

int Dg_TclCommand(ClientData,
		  Tcl_Interp *interp, 
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
    strcpy(interp->result, visi_MetricName(m));
    return TCL_OK;

  case METRICUNITS:  
    m = atoi(argv[2]);
    strcpy(interp->result, visi_MetricLabel(m));
    return TCL_OK;

  case METRICAVEUNITS:  
    m = atoi(argv[2]);
    strcpy(interp->result, visi_MetricAveLabel(m));
    return TCL_OK;

  case METRICSUMUNITS:  
    m = atoi(argv[2]);
    strcpy(interp->result, visi_MetricSumLabel(m));
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
    strcpy(interp->result, visi_ResourceName(r));
    return TCL_OK;

  case STARTSTREAM:       
    visi_GetMetsRes(argv[2],0); 
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
//    sprintf(interp->result, "%d", Enabled(m,r));
    return TCL_OK;

  case VALUE:       
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    buck = atoi(argv[4]);
    sprintf(interp->result,"%g", visi_DataValue(m,r,buck));
    return TCL_OK;
  }

  sprintf(interp->result, "Internal error (func findCommand)\n");
  return TCL_ERROR;
}

void (*UsersNewDataCallbackRoutine)(int firstBucket, int lastBucket);
   // we will call this routine for you when we get a new-data callback
   // from the visi lib (first, we do a bit of processing for you, such
   // as determining what the range is buckets you haven't seen yet is).

int Dg2_Init(Tcl_Interp *interp) {
   // initialize with the visi lib
   int fd = visi_Init();
   if (fd < 0) {
      cerr << "Dg2_Init() -- could not initialize with the visi lib" << endl;
      exit(5);
   }

   // Register C++ Callback routines with the visi lib when
   // certain events happen.  The most important (performance-wise)
   // is the DATAVALUES callback, which signals the arrival of
   // new barchart data.  We must process this callback very quickly,
   // in order to perturb the system as little as possible.

   if (visi_RegistrationCallback(ADDMETRICSRESOURCES,Dg2AddMetricsCallback)!=0)
      panic("Dg2_Init() -- couldn't install ADDMETRICSRESOURCES callback");

   if (visi_RegistrationCallback(FOLD, Dg2Fold) != 0)
      panic("Dg2_Init() -- couldn't install FOLD callback");

   if (visi_RegistrationCallback(INVALIDMETRICSRESOURCES, Dg2InvalidMetricsOrResources) != 0)
      panic("Dg2_Init() -- couldn't install INVALID callback");

   if (visi_RegistrationCallback(PHASESTART, Dg2PhaseNameCallback) != 0)
      panic("Dg2_Init() -- couldn't install PHASENAME callback");

   if (visi_RegistrationCallback(DATAVALUES, Dg2NewDataCallback) != 0)
      panic("Dg2_Init() -- couldn't install DATAVALUES callback");

   // install "Dg" as a new tcl command; Dg_TclCommand() will be invoked when
   // a tcl script calls Dg
   Tcl_CreateCommand(interp, "Dg", Dg_TclCommand, 
		    (ClientData *) NULL,(Tcl_CmdDeleteProc *) NULL);
 
   // Arrange for my_visi_callback() to be called whenever data is waiting
   // to be read off of descriptor "fd".  Extremely important! [tcl book
   // page 357]
   Tk_CreateFileHandler(fd, TK_READABLE, (Tk_FileProc *) my_visi_callback, 0);

   return TCL_OK;
}
