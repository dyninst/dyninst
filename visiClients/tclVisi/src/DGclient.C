/*
 *  DGclient.C -- Code for the visi<->tcl interface.
 *    
 * $Log: DGclient.C,v $
 * Revision 1.7  1995/02/26 02:02:02  newhall
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
#include "../../../visi/h/visualization.h"

extern Tcl_Interp *MainInterp;

void my_visi_callback(void* arg0, int* arg1, long unsigned int* arg2) {
    if (-1 == visi_callback())
       exit(1);
}

int Dg_Add(int dummy) {
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

int Dg_Fold(int dummy) {
   const int retval=Tcl_Eval(MainInterp, "DgFoldCallback");
   if (retval == TCL_ERROR)
      cerr << MainInterp->result << endl;

   return retval;
}

int Dg_Invalid(int dummy) {
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

int Dg_PhaseData(int dummy) {
   const int retval=Tcl_Eval(MainInterp, "DgPhaseDataCallback");
   if (retval == TCL_ERROR)
      cerr << MainInterp->result << endl;

  return retval;
}

#define   AGGREGATE        0
#define   BINWIDTH         1
#define   FOLDMETHOD       2
#define   METRICNAME       3
#define   METRICUNITS      4
#define   NUMBINS          5
#define   NUMMETRICS       6
#define   NUMRESOURCES     7
#define   DEFINEPHASE      8
#define   RESOURCENAME     9
#define   STARTSTREAM      10
#define   STOPSTREAM       11
#define   DGSUM            12
#define   DGVALID          13
#define   DGENABLED        14
#define   VALUE            15
#define   CMDERROR         16
#define   LASTBUCKET       17
#define   FIRSTBUCKET      18
#define   NUMPHASES        19
#define   PHASENAME        20
#define   PHASESTARTTIME   21
#define   PHASEENDTIME     22

struct cmdTabEntry {
   char *cmdname;
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
  PhaseInfo *p;

  switch(cmdDex) {
  case AGGREGATE:   
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    sprintf(interp->result,"%g", dataGrid.AggregateValue(m,r));
    return TCL_OK;

  case BINWIDTH:     
    sprintf(interp->result, "%g", dataGrid.BinWidth());
    return TCL_OK;

  case FIRSTBUCKET:
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    sprintf(interp->result,"%d", dataGrid[m][r].FirstValidBucket()); 
    return TCL_OK;

  case FOLDMETHOD:
    m = atoi(argv[2]);
    sprintf(interp->result,"%d", dataGrid.FoldMethod(m));
    return TCL_OK;

  case LASTBUCKET:
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    sprintf(interp->result,"%d", dataGrid[m][r].LastBucketFilled());
    return TCL_OK;

  case METRICNAME:  
    m = atoi(argv[2]);
    sprintf(interp->result, "%s", dataGrid.MetricName(m));
    return TCL_OK;

  case METRICUNITS:  
    m = atoi(argv[2]);
    sprintf(interp->result, "%s", dataGrid.MetricUnits(m));
    return TCL_OK;

  case NUMBINS:     
    sprintf(interp->result, "%d", dataGrid.NumBins());
    return TCL_OK;

  case NUMMETRICS:  
    sprintf(interp->result, "%d", dataGrid.NumMetrics());
    return TCL_OK;

  case NUMRESOURCES:
    sprintf(interp->result, "%d", dataGrid.NumResources());
    return TCL_OK;

  case DEFINEPHASE:       
    // DefinePhase(atof(argv[2]), argv[3]);
    DefinePhase(-1.0, NULL);
    return TCL_OK;

  case RESOURCENAME:
    r = atoi(argv[2]);
    sprintf(interp->result, "%s", dataGrid.ResourceName(r));
    return TCL_OK;

  case STARTSTREAM:       
    // GetMetsRes(argv[2], argv[3], 0); 
//    GetMetsRes((char *)NULL,0, 0); 
    GetMetsRes(argv[2], atoi(argv[3]), 0); // 0-->histogram (1-->scalar)
                                           // argv[3] is num
    return TCL_OK;

  case STOPSTREAM:
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    StopMetRes(m, r);
    return TCL_OK;

  case DGSUM:         
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    sprintf(interp->result,"%g", dataGrid.SumValue(m,r));
    return TCL_OK;

  case DGVALID:
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    sprintf(interp->result, "%d", dataGrid.Valid(m,r));
    return TCL_OK;

  case DGENABLED:
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    sprintf(interp->result, "%d", dataGrid[m][r].Enabled());
    return TCL_OK;

  case VALUE:       
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    buck = atoi(argv[4]);
    sprintf(interp->result,"%g", dataGrid[m][r].Value(buck));
    return TCL_OK;

  case NUMPHASES:
    sprintf(interp->result, "%d", dataGrid.NumPhases());
    return TCL_OK;

  case PHASENAME:
    m = atoi(argv[2]);
    p = dataGrid.GetPhaseInfo(m);
    sprintf(interp->result, "%s", p->getName());
    return TCL_OK;

  case PHASESTARTTIME:
    m = atoi(argv[2]);
    p = dataGrid.GetPhaseInfo(m);
    sprintf(interp->result, "%f", p->getStartTime());
    return TCL_OK;

  case PHASEENDTIME:
    m = atoi(argv[2]);
    p = dataGrid.GetPhaseInfo(m);
    sprintf(interp->result, "%f", p->getEndTime());
    return TCL_OK;

  }

  sprintf(interp->result, "Internal error (func findCommand)\n");
  return TCL_ERROR;
}

int Dg_Init(Tcl_Interp *interp) {
   int fd=VisiInit();
   if (fd < 0) {
      cerr << "tclVisi: could not initialize visilib" << endl;
      exit(-1);
   }

  (void) RegistrationCallback(ADDMETRICSRESOURCES,Dg_Add); 
  (void) RegistrationCallback(DATAVALUES,Dg_Data); 
  (void) RegistrationCallback(FOLD,Dg_Fold); 
  (void) RegistrationCallback(INVALIDMETRICSRESOURCES,Dg_Invalid);
  (void) RegistrationCallback(PHASESTART,Dg_PhaseStart);
  (void) RegistrationCallback(PHASEEND,Dg_PhaseEnd);
  (void) RegistrationCallback(PHASEDATA,Dg_PhaseData);

  Tcl_CreateCommand(interp, "Dg", Dg_TclCommand, 
		    (ClientData *) NULL,(Tcl_CmdDeleteProc *) NULL);
 
  // Arrange for my_visi_callback() to be called whenever data is waiting
  // to be read off of descriptor "fd".  Extremely important! [tcl book
  // page 357]
  Tk_CreateFileHandler(fd, TK_READABLE, (Tk_FileProc *) my_visi_callback, 0);

  return TCL_OK;
}


