/*
 *  DGclient.C -- Code for the visi<->tcl interface.
 *    
 * $Log: DGclient.C,v $
 * Revision 1.3  1994/08/05 20:17:10  rbi
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
#include <tcl.h>
#include <tk.h>
#include "visi/h/visualization.h"

extern "C" {
  int Blt_GraphElement(Tcl_Interp *interp, char *pathName, char *elemName, 
		       int numValues, double *valueArr);
}

extern Tcl_Interp *MainInterp;

int Dg_Add(int dummy) {
  int retval;

  retval = Tcl_Eval(MainInterp, "DgConfigCallback");
  if (retval == TCL_ERROR) {
    fprintf(stderr, "%s\n", MainInterp->result);
  }
  return(retval);
}

int Dg_Data(int dummy) {
  int retval = TCL_OK, thislast = -1;
  static LastBucket = 0;
  char cmd[256];

  /* 
   *  Simulate valid callback
   */
  for (unsigned m = 0; m < dataGrid.NumMetrics(); m++) {
    for (unsigned r = 0; r < dataGrid.NumResources(); r++) {
      if(dataGrid[m][r].Valid){
	if (! dataGrid[m][r].userdata) {
	  sprintf(cmd,"DgValidCallback %d %d", m, r);
	  retval = Tcl_Eval(MainInterp, cmd);
	  if (retval == TCL_ERROR) {
	    fprintf(stderr, "%s\n", MainInterp->result);
	  }
	  dataGrid[m][r].userdata = (void *) malloc(sizeof (int));
	  *((int *) dataGrid[m][r].userdata) = 1;
	}
	if (thislast < 0) {
	  thislast = dataGrid[m][r].LastBucketFilled();
	}
      }
    }
  }

  /*
   *  Send range to tcl
   */
  if (thislast < LastBucket) {
    LastBucket = thislast-1;
  }
  if (thislast >= 0) {
    sprintf(cmd,"DgDataCallback %d %d", LastBucket+1, thislast);
    retval = Tcl_Eval(MainInterp, cmd);
    if (retval == TCL_ERROR) {
      fprintf(stderr, "%s\n", MainInterp->result);
    }
    LastBucket = thislast;
  }

  return(retval);
}

int Dg_Fold(int dummy) {
  int retval;

  retval = Tcl_Eval(MainInterp, "DgFoldCallback");
  if (retval == TCL_ERROR) {
    fprintf(stderr, "%s\n", MainInterp->result);
  }
  return(retval);
}

int Dg_Invalid(int dummy) {
  int retval;

  retval = Tcl_Eval(MainInterp, "DgInvalidCallback");
  if (retval == TCL_ERROR) {
    fprintf(stderr, "%s\n", MainInterp->result);
  }
  return(retval);
}

int Dg_Phase(int dummy) {
  int retval;

  retval = Tcl_Eval(MainInterp, "DgPhaseCallback");
  if (retval == TCL_ERROR) {
    fprintf(stderr, "%s\n", MainInterp->result);
  }
  return(retval);
}

int Dg_GraphElem(Tcl_Interp *interp, char *path, char *elem, int mid, int rid) 
{
  static double *coords = NULL;
  double *cptr, bwid;
  float *vals, *vptr;
  int numb, b;

  /* Allocate an array for the coords */
  if (coords == NULL) {
    numb = dataGrid.NumBins();
    coords = (double *) malloc(sizeof(double) * numb * 2);
    if (!coords) {
      sprintf(interp->result, "Dg_GraphElem: Could not allocate coords\n");
      return TCL_ERROR;
    }
  }    

  /* Binwidth and numbuckets give us the t coords */
  numb = dataGrid[mid][rid].LastBucketFilled()+1;
  bwid = dataGrid.BinWidth();

  /* Get the data values */
  vals = dataGrid[mid][rid].Value();

  /* Fill the array */
  cptr = coords;  
  vptr = vals;
  for (b = 0; b < numb; b++) {
    *cptr++ = b*bwid;
    if (isnan(*vptr)) {
      *cptr = 0.0;
    } else {
      *cptr = (double) *vptr;
    }
    cptr++;
    vptr++;
  }

  /* Give it to BLT */
  return(Blt_GraphElement(interp, path, elem, numb*2, coords));
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
#define   VALUE            14
#define   CMDERROR         15
#define   LASTBUCKET       16
#define   BLTGRAPHELEM     17
#define   FIRSTBUCKET      18

struct cmdTabEntry 
{
  char *cmdname;
  int index;
  int numargs;
};

static struct cmdTabEntry Dg_Cmds[] = {
  {"aggregate",    AGGREGATE,       2},
  {"binwidth",     BINWIDTH,        0},
  {"bltgraphelem", BLTGRAPHELEM,    4},
  {"firstbucket",  FIRSTBUCKET,      2},
  {"foldmethod",   FOLDMETHOD,      2},
  {"lastbucket",   LASTBUCKET,      2},
  {"metricname",   METRICNAME,      1},
  {"metricunits",  METRICUNITS,     1},
  {"numbins",      NUMBINS,         0},
  {"nummetrics",   NUMMETRICS,      0},
  {"numresources", NUMRESOURCES,    0},
  {"phase",        DEFINEPHASE,     3},
  {"resourcename", RESOURCENAME,    1},
  {"start",        STARTSTREAM,     2},
  {"stop",         STOPSTREAM,      2},
  {"sum",          DGSUM,           2},
  {"valid",        DGVALID,         2},
  {"value",        VALUE,           3},
  {NULL,           CMDERROR,        0}
};

static int findCommand(Tcl_Interp *interp, 
		       int argc, 
		       char *argv[])
{
  struct cmdTabEntry *C;

  if (argc == 0) {
    sprintf(interp->result, "USAGE: Dg <option> [args...]\n");
    return CMDERROR;
  }
  for (C = Dg_Cmds; C->cmdname; C++) {
    if (strcmp(argv[0], C->cmdname) == 0) {
      if ((argc-1) == C->numargs) 
	return C->index;
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
	       char *argv[])
{
  int cmdDex, m, r, buck;

  cmdDex = findCommand(interp, argc-1, argv+1);
  if (cmdDex == CMDERROR) {
    return TCL_ERROR;
  }

  switch(cmdDex) {
  case AGGREGATE:   
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    sprintf(interp->result,"%g", dataGrid.AggregateValue(m,r));
    return TCL_OK;

  case BINWIDTH:     
    sprintf(interp->result, "%g", dataGrid.BinWidth());
    return TCL_OK;

  case BLTGRAPHELEM:     
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    return (Dg_GraphElem(interp, argv[4], argv[5], m, r));

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
    NamePhase(atof(argv[2]), atof(argv[3]), argv[4]);
    return TCL_OK;

  case RESOURCENAME:
    r = atoi(argv[2]);
    sprintf(interp->result, "%s", dataGrid.ResourceName(r));
    return TCL_OK;

  case STARTSTREAM:       
    GetMetsRes(argv[2], argv[3], 0);
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

  case VALUE:       
    m = atoi(argv[2]);
    r = atoi(argv[3]);
    buck = atoi(argv[4]);
    sprintf(interp->result,"%g", dataGrid[m][r].Value(buck));
    return TCL_OK;
  }

  sprintf(interp->result, "Internal error (func findCommand)\n");
  return TCL_ERROR;
}

static void
my_visi_callback(void* arg0, int* arg1, long unsigned int* arg2)
{
    int ret;

    ret = visi_callback();
    if (ret == -1) exit(0);
}

int 
Dg_Init(Tcl_Interp *interp)
{
  int fd;

  /* Initialize visualization module */
  if((fd = VisiInit()) < 0){
    exit(-1);
  }
  (void) RegistrationCallback(ADDMETRICSRESOURCES,Dg_Add); 
  (void) RegistrationCallback(DATAVALUES,Dg_Data); 
  (void) RegistrationCallback(FOLD,Dg_Fold); 
  (void) RegistrationCallback(INVALIDMETRICSRESOURCES,Dg_Invalid);
  (void) RegistrationCallback(PHASENAME,Dg_Phase);
  {
    /* char *vargv[3];*/
    /* vargv[0] = "foo";*/
    /* vargv[1] = "cpu";*/
    /* vargv[2] = "Procedure Process Machine SyncObject";*/

    /* (void) StartVisi(3,vargv);*/
    (void) StartVisi(0,NULL); 
  }

  Tcl_CreateCommand(interp, "Dg", Dg_TclCommand, 
		    (ClientData *) NULL,(Tcl_CmdDeleteProc *) NULL);
 
  Tk_CreateFileHandler(fd, TK_READABLE, (Tk_FileProc *) my_visi_callback, 0);

  return TCL_OK;
}


