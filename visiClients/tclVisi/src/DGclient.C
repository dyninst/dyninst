/*
 *  DGclient.C -- Code for the visi<->tcl interface.
 *    
 * $Log: DGclient.C,v $
 * Revision 1.11  1996/01/19 20:56:29  newhall
 * changes due to visiLib interface changes
 *
 * Revision 1.10  1996/01/17 18:32:34  newhall
 * changes due to new visiLib
 *
 * Revision 1.9  1995/11/17  17:30:32  newhall
 * added Dg metriclabel, Dg metricavelabel, and Dg metricsumlabel commands
 * changed the Dg start command so that it doesn't take any arguments
 *
 * Revision 1.8  1995/11/12  23:30:49  newhall
 * added Dg_Exited
 *
 * Revision 1.7  1995/02/26  02:02:02  newhall
 * added callback functions for new visiLib phase info.
 *
 * Revision 1.6  1994/11/08  00:20:26  tamches
 * removed blt-ish influences
 * sped up processing of new data callbacks
 * very close now to dg2.C of barchart
 *
 * Revision 1.5  1994/09/30  21:03:07  newhall
 * removed call to StartVisi
 *
 * Revision 1.4  1994/09/25  02:07:47  newhall
 * changed arguments to GetMetsRes
 *
 * Revision 1.3  1994/08/05  20:17:10  rbi
 * Update for new version of libvisi.a
 *
 * Revision 1.2  1994/06/14  18:57:47  rbi
 * Updated layout and added curve validation callback.
 *
 * Revision 1.1  1994/05/31  21:05:47  rbi
 * Initial version of tclVisi and tabVis
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
#define   METRICAVELAB     22
#define   METRICSUMLAB	   23

struct cmdTabEntry {
   char *cmdname;
   int index;
   int numargs;
};

static struct cmdTabEntry Dg_Cmds[] = {
  {"aggregate",    AGGREGATE,       2},
  {"binwidth",     BINWIDTH,        0},
  {"firstbucket",  FIRSTBUCKET,     2},
  {"lastbucket",   LASTBUCKET,      2},
  {"metricname",   METRICNAME,      1},
  {"metriclabel",  METRICUNITS,     1},
  {"numbins",      NUMBINS,         0},
  {"nummetrics",   NUMMETRICS,      0},
  {"numresources", NUMRESOURCES,    0},
  {"phase",        DEFINEPHASE,     0},
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
  {"numphases",    NUMPHASES,       0},
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

int Dg_TclCommand(ClientData clientData,
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

  case DEFINEPHASE:       
    visi_DefinePhase(NULL);
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
  // to be read off of descriptor "fd".  Extremely important! [tcl book
  // page 357]
  Tk_CreateFileHandler(fd, TK_READABLE, (Tk_FileProc *) my_visi_callback, 0);

  return TCL_OK;
}


