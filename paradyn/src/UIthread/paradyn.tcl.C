/* paradyn.tcl.C

   This code implements the tcl "paradyn" command.  See the README file for 
   command descriptions.

*/
/* $Log: paradyn.tcl.C,v $
/* Revision 1.41  1995/06/02 20:50:43  newhall
/* made code compatable with new DM interface
/*
 * Revision 1.40  1995/02/27  18:56:48  tamches
 * Changes to reflect the new TCthread.
 *
 * Revision 1.39  1995/02/16  08:20:52  markc
 * Changed Boolean to bool
 * Changed wait loop code for igen messages
 *
 * Revision 1.38  1995/02/07  21:52:54  newhall
 * changed parameters to VMCreateVisi call
 *
 * Revision 1.37  1995/01/26  17:59:04  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.36  1994/12/21  07:39:51  tamches
 * Removed uses of tunableConstant::allConstants, which became a private
 * class variable.
 *
 * Revision 1.35  1994/12/21  00:43:14  tamches
 * used the new findTunableConstant() method function, instead of doing it
 * by looking into tc's data members (which is no longer allowed).
 *
 * Revision 1.34  1994/11/11  15:12:35  rbi
 * causing serious illness to debugging printf()
 *
 * Revision 1.33  1994/11/10  17:35:57  rbi
 * physical illness and possible death in the family
 *
 * Revision 1.32  1994/11/07  08:25:14  jcargill
 * Added ability to suppress search on children of a resource, rather than
 * the resource itself.
 *
 * Revision 1.31  1994/11/04  16:29:08  rbi
 * Added paradyn daemon command
 *
 * Revision 1.30  1994/11/03  20:25:08  krisna
 * added status_lines for application name and application status
 *
 * Revision 1.29  1994/11/01  05:42:35  karavan
 * some minor performance and warning fixes
 *
 * Revision 1.28  1994/09/25  01:54:12  newhall
 * updated to support changes in VM, and UI interface
 *
 * Revision 1.27  1994/09/22  01:17:53  markc
 * Cast stringHandles to char*s in printf statements
 *
 * Revision 1.26  1994/08/22  15:55:29  markc
 * Added extra argument to addExecutable call.
 *
 * Revision 1.25  1994/08/13  20:55:33  newhall
 * changed call to VMCreateVisi
 *
 * Revision 1.24  1994/08/08  20:15:25  hollings
 * added suppress instrumentation command.
 *
 * Revision 1.23  1994/08/05  16:04:28  hollings
 * more consistant use of stringHandle vs. char *.
 *
 * Revision 1.22  1994/08/03  19:10:25  hollings
 * split tunable constant into boolean and float types.
 *
 * Revision 1.21  1994/07/25  14:58:15  hollings
 * added suppress resource option.
 *
 * Revision 1.20  1994/07/07  03:27:36  markc
 * Changed expected result of call to dataMgr->addExecutable
 *
 * Revision 1.19  1994/07/03  05:00:24  karavan
 * bug fix: removed call to delete name returned from getCanonicalName()
 *
 * Revision 1.18  1994/07/02  01:44:13  markc
 * Removed aggregation operator from enableDataCollection call.
 *
 * Revision 1.17  1994/06/14  15:20:17  markc
 * Added extra arg to enableDataCollection call.  This is probably temporaray
 * since the data manager or configuration language will specify this info.
 *
 * Revision 1.16  1994/05/31  19:11:49  hollings
 * Changes to permit direct access to resources and resourceLists.
 *
 * Revision 1.15  1994/05/26  21:26:10  karavan
 * corrected return value for Process command, to return TCL_ERROR if call
 * to Add_Executable fails.
 *
 * Revision 1.14  1994/05/18  00:50:12  hollings
 * added pid argument to core command.
 *
 * Revision 1.13  1994/05/12  23:34:16  hollings
 * made path to paradyn.h relative.
 *
 * Revision 1.12  1994/05/09  20:59:27  hollings
 * Changed paradyn shg start to clearSHG not init it.
 *
 * Revision 1.11  1994/05/06  06:40:06  karavan
 * added shg start command
 *
 * Revision 1.10  1994/05/05  02:13:29  karavan
 * moved CmdTabEntry definition from paradyn.tcl.C to UIglobals.h
 *
 * Revision 1.9  1994/05/02  20:38:31  hollings
 * added search pause command and shg commands.
 *
 * Revision 1.8  1994/04/27  22:55:09  hollings
 * Merged refine auto and search.
 *
 * Revision 1.7  1994/04/21  23:24:51  hollings
 * added process command.
 *
 * Revision 1.6  1994/04/19  22:09:14  rbi
 * Added new tcl commands and updated "enable" to return met id
 *
 * Revision 1.5  1994/04/10  19:12:12  newhall
 * added visi command
 *
 * Revision 1.4  1994/04/09  18:37:20  hollings
 * Fixed paramter to tunable constant to work.
 *
 * Revision 1.3  1994/04/06  22:40:15  markc
 * Included assert.h.
 *
 * Revision 1.2  1994/04/05  23:49:25  rbi
 * Fixed a bunch of tcl related stuff.
 *
 * Revision 1.1  1994/04/05  04:42:38  karavan
 * initial version of UI thread code and tcl paradyn command
 * */

/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */

#include <string.h>
#include "UIglobals.h"
#include "paradyn/src/DMthread/DMinclude.h"
#include "../TCthread/tunableConst.h"
#include "VM.thread.CLNT.h"
#include "thread/h/thread.h"
#include "../pdMain/paradyn.h"
#include <assert.h>
#include <stdlib.h>

#include "Status.h"

extern bool detachApplication(bool);

int ParadynPauseCmd(ClientData clientData, 
		Tcl_Interp *interp, 
		int argc, 
		char *argv[])
{
  dataMgr->pauseApplication();
  return TCL_OK;
}


int ParadynContCmd(ClientData clientData, 
		Tcl_Interp *interp, 
		int argc, 
		char *argv[])
{
  dataMgr->continueApplication();
  return TCL_OK;
}

int ParadynStatusCmd(ClientData clientData, 
		Tcl_Interp *interp, 
		int argc, 
		char *argv[])
{
  dataMgr->printStatus();
  return TCL_OK;
}

int ParadynMetricsCmd(ClientData clientData, 
			Tcl_Interp *interp, 
			int argc, 
			char *argv[])
{
  vector<string> *ml;
  int i;
  
  ml = dataMgr->getAvailableMetrics();
  for (i=0; i < ml->size(); i++)
    Tcl_AppendElement(interp, (char *)((*ml)[i]).string_of());
  delete ml;
  return TCL_OK;
}


int ParadynDaemonsCmd(ClientData clientData, 
		      Tcl_Interp *interp, 
		      int argc, 
		      char *argv[])
{
  vector<string> *dl;
  
  dl = dataMgr->getAvailableDaemons();
  for (int i=0; i < dl->size(); i++)
    Tcl_AppendElement(interp, (char *)((*dl)[i]).string_of());
  delete dl;
  return TCL_OK;
}


int ParadynResourcesCmd(ClientData clientData, 
			Tcl_Interp *interp, 
			int argc, 
			char *argv[])
{
  dataMgr->printResources();
  return TCL_OK;
}

int ParadynListCmd(ClientData clientData, 
		Tcl_Interp *interp, 
		int argc, 
		char *argv[])
{
  vector<string> *ml;
  int i;

  dataMgr->printResources();
  ml = dataMgr->getAvailableMetrics();
  for (i=0; i < ml->size(); i++) {
    cout << ((*ml)[i]).string_of() << endl;
  }

  cout << "CONSTANTS" << endl;

  vector<tunableBooleanConstant> allBoolConstants = tunableConstantRegistry::getAllBoolTunableConstants();
  for (int boollcv = 0; boollcv < allBoolConstants.size(); boollcv++) {
     tunableBooleanConstant &tbc = allBoolConstants[boollcv];
     tbc.print();
  }

  vector<tunableFloatConstant> allFloatConstants = tunableConstantRegistry::getAllFloatTunableConstants();
  for (int floatlcv = 0; floatlcv < allFloatConstants.size(); floatlcv++) {
     tunableFloatConstant &tfc = allFloatConstants[floatlcv];
     tfc.print();
  }

  cout << "bucketWidth " << dataMgr->getCurrentBucketWidth() << endl;
  cout << "number of buckets = " << dataMgr->getMaxBins() << endl;
  dataMgr->printDaemons();
  return TCL_OK;
}

int ParadynDetachCmd (ClientData clientData,
		      Tcl_Interp *interp,
		      int argc,
		      char *argv[])
{
  dataMgr->detachApplication(true);
  return TCL_OK;
}

metricHandle *
StrToMetHandle (char *mstr)
{
  metricHandle *mh = new metricHandle;
  if (sscanf (mstr, "%u", mh) <= 0) {
    delete mh;
    return (metricHandle *) NULL;
  }
  else return mh;
}

char *
MetHandleToStr (metricHandle mh)
{
  char *result = new char[12];
  sprintf (result, "%u", mh);
  return result;
}

int ParadynGetTotalCmd (ClientData clientData,
		     Tcl_Interp *interp,
		     int argc,
		     char *argv[])
{
  metricHandle *met;
  metricInstInfo *mi;
  float val;

  if (argc < 2) {
    sprintf(interp->result, "USAGE: gettotal <metid>");
    return TCL_ERROR;
  }

  if (!(met = dataMgr->findMetric (argv[1]))) {
    Tcl_AppendElement (interp, "invalid metric identifier");
    return TCL_ERROR;
  }
  
  mi = uim_enabled.find((void *) *met);
  if (!mi) {
    Tcl_AppendResult (interp, "unable to find metric ", MetHandleToStr(*met),
		      (char *)NULL);
    delete met;
    return TCL_ERROR;
  }
  else {
    val = dataMgr->getTotValue(mi->mi_id);
    sprintf(interp->result, "%g", val);
    delete met;
  }  
  return TCL_OK;
}

int ParadynPrintCmd (ClientData clientData,
		     Tcl_Interp *interp,
		     int argc,
		     char *argv[])
{
  if (argv[1][0] == 'm') {   // print metric
    float val;
    metricInstInfo *mi;
    metricHandle *met;

    if (! (met = dataMgr->findMetric (argv[2]))) {
      Tcl_AppendElement (interp, "Invalid metric");
      return TCL_ERROR;
    }
    mi = uim_enabled.find((void *) *met);

    if (!mi) {
      sprintf (interp->result, "unable to find metric %s\n", 
	       argv[2]);
      delete met;
      return TCL_ERROR;
     } else {
      val = dataMgr->getMetricValue(mi->mi_id);
      printf ("metric %s, val = %f\n", 
	       (char*)dataMgr->getMetricName(*met), val);
    }
  } else if (argv[1][0] == 's') {     //print shg
      perfConsult->printSHGList();
  } else if (argv[1][0] == 'r') {     // print refine
    int i;
    searchHistoryNode *currentSHGNode;
    SHNptr_Array currentRefinementList;
	
    currentSHGNode = perfConsult->getCurrentRefinement();
    currentRefinementList = perfConsult-> getAllRefinements (currentSHGNode);

    for (i=0; i < currentRefinementList.count; i++) {
      currentSHGNode = currentRefinementList.data[i];
      perfConsult->printSHGNode(currentSHGNode);
      printf ("\n");
    }
  } else {
    sprintf (interp->result, "Unknown option: paradyn print %s\n", argv[1]);
    return TCL_ERROR;
  }
  return TCL_OK;
}

void processUsage()
{
  printf("USAGE: process <-user user> <-machine machine> <-daemon> daemon> \"command\"\n");
}

/****
 * Process
 * Calls data manager service "addExecutable".  
 * Returns TCL_OK or TCL_ERROR
 */
int ParadynProcessCmd(ClientData clientData,
		      Tcl_Interp *interp,
		      int argc,
		      char *argv[])
{
  int i;
  char *user = NULL;
  char *machine = NULL;
  char *paradynd = NULL;
  
  for (i=1; i < argc-1; i++) {
    if (!strcmp("-user", argv[i])) {
      if (i+1 == argc) {
	processUsage();
	return TCL_ERROR;
      }
      user = argv[++i];
    } else if (!strcmp("-machine", argv[i])) {
      if (i+1 == argc) {
	processUsage();
	return TCL_ERROR;
      }
      machine = argv[++i];
    } else if (!strcmp("-daemon", argv[i])) {
      if (i+1 == argc) {
	processUsage();
	return TCL_ERROR;
      }
      paradynd = argv[++i];
    } else if (argv[i][0] != '-') {
      break;
    } else {
      processUsage();
      return TCL_ERROR;
    }
  }

  static status_line app_name("Application name");
  static char tmp_buf[1024];
  sprintf(tmp_buf, "program: %s, machine: %s, user: %s, daemon: %s",
	  argv[i], machine?machine:"(local)", user?user:"(self)",
	  paradynd?paradynd:"(default)");
  app_name.message(tmp_buf);
  
  vector<string> av;
  unsigned ve=i;
  while (argv[ve]) {
    av += argv[ve];
    ve++;
  }
  if (dataMgr->addExecutable(machine, user, paradynd, (char*)0,
			     &av) == false)
    return TCL_ERROR;
  else
    return TCL_OK;
}

//
//  disable  <metid>
//
int ParadynDisableCmd (ClientData clientData,
		      Tcl_Interp *interp,
		      int argc,
		      char *argv[])
{
  metricHandle *met;
  metricInstInfo *mi;

  // Hold Everything!
  dataMgr->pauseApplication ();

  if (argc < 2) {
    sprintf(interp->result, "USAGE: disable <metid>");
    return TCL_ERROR;
  }

  if (! (met = dataMgr->findMetric(argv[1]))) {
    sprintf(interp->result, "Invalid metric %s", argv[1]);
    return TCL_ERROR;
  }

  mi = uim_enabled.find((void *) *met);

  if (!mi) {
    sprintf (interp->result, "unable to find metric %s\n", 
	     MetHandleToStr(*met)); 
    delete met;
    return TCL_ERROR;
  }
  else {
    dataMgr->disableDataCollection (uim_ps_handle, mi->mi_id);
    delete met;
  }
  return TCL_OK;
}

//
//  enable <metric> ?<resource>? ...
//    returns metric id
//
int ParadynEnableCmd (ClientData clientData,
		      Tcl_Interp *interp,
		      int argc,
		      char *argv[])
{
  metricHandle *met;
  metricInstInfo *mi;
  vector<resourceHandle> *resList;

  // Hold Everything!
  dataMgr->pauseApplication ();

  // Build a resource list from the tcl list
  if (argc == 2)
    resList = dataMgr->getRootResources();
  else {
    char **argsv;
    int argsc;
    resourceHandle *res;

    if (Tcl_SplitList(interp, argv[2], &argsc, &argsv) != TCL_OK) {
      printf("Error parsing resource list '%s'", argv[2]);
      return TCL_ERROR;
    }

    resList = new vector<resourceHandle>;
    cout << "enable request for ";
    for (int i = 0; i < argsc; i++) {
      res = dataMgr->findResource(argsv[i]);
      cout << argsv[i] << " ";
      resList += *res;
    }
    cout << endl;
    free(argsv);
  }

  // Now check the metric
  met = dataMgr->findMetric (argv[1]);
  if (!met) {
    sprintf (interp->result, "metric %s is not defined\n", argv[1]);
    delete resList;
    return TCL_ERROR;
  }
  else {
    // Finally enable the data collection
    mi = dataMgr->enableDataCollection (uim_ps_handle, resList, *met);
    if (mi) {
      uim_enabled.add(mi, (void *)mi->mi_id);
      sprintf(interp->result, MetHandleToStr (mi->mi_id));
      printf ("metric %s, id = %s\n", argv[1], MetHandleToStr(mi->mi_id));
    } else {
      sprintf (interp->result, "can't enable metric %s for focus \n", argv[1]);
      return TCL_ERROR;
    }
  }
  return TCL_OK;
}

int ParadynCoreCmd (ClientData clientData,
		    Tcl_Interp *interp,
		    int argc,
		    char *argv[])
{
  int pid;

  if (argc != 2) {
    printf("usage: paradyn core <pid>\n");
    return TCL_ERROR;
  }
  if (sscanf(argv[1],"%d",&pid) != 1) {
    printf("usage: paradyn core <pid>\n");
    return TCL_ERROR;
  }

  dataMgr->coreProcess(pid);
  return TCL_OK;
}

int ParadynSetCmd (ClientData clientData,
		    Tcl_Interp *interp,
		    int argc,
		    char *argv[])
{
  if (argc != 3) {
    sprintf(interp->result,"USAGE: %s <variable> <value>", argv[0]);
    return TCL_ERROR;
  }

  if (!tunableConstantRegistry::existsTunableConstant(argv[1])) {
     cout << "Tunable constant " << argv[1] << " does not exist; cannot set its value to " << argv[2] << endl;
     return TCL_ERROR;
  }
  
  if (tunableConstantRegistry::getTunableConstantType(argv[1]) == tunableBoolean) {
     int boolVal;
     if (TCL_ERROR == Tcl_GetBoolean(interp, argv[2], &boolVal))
        return TCL_ERROR;
     else {
        tunableConstantRegistry::setBoolTunableConstant(argv[1], (bool)boolVal);
        cout << "tunable boolean constant " << argv[1] << " set to " << boolVal << endl;
     }
  }
  else {
     double doubleVal;
     if (TCL_ERROR == Tcl_GetDouble(interp, argv[2], &doubleVal))
        return TCL_ERROR;
     else {
        tunableConstantRegistry::setFloatTunableConstant(argv[1], (float)doubleVal);
        cout << "tunable float constant " << argv[1] << " set to " << doubleVal << endl;
     }
  }

  return TCL_OK;
}

int ParadynSearchCmd (ClientData clientData,
		      Tcl_Interp *interp,
		      int argc,
		      char *argv[])
{
  int limit;

  if (argc == 2 && !strcmp(argv[1], "pause")) {
    // stop the search
    perfConsult->pauseSearch();
    return TCL_OK;
  } else if (argc == 3) {
    if (Tcl_GetInt (interp, argv[2], &limit) == TCL_ERROR) 
      return TCL_ERROR;
  } else if (argc == 2) {
    limit = -1;
  } else {
    printf("Usage: paradyn search <false|true> <int>\n");
    printf("       paradyn search pause\n");
    return TCL_ERROR;
  }

  if (dataMgr->applicationDefined() != true) {
    sprintf (interp->result, "no program defined, can't search\n");
    return TCL_ERROR;
  } else {
    perfConsult->search(true, limit);
    return TCL_OK;
  }
}


int ParadynSHGCmd (ClientData clientData,
		   Tcl_Interp *interp,
		   int argc,
		   char *argv[])
{
  int node;

  if (argc == 2 && !strcmp(argv[1], "get")) {
    node = perfConsult->getCurrentNodeId();
    sprintf(interp->result, "%d", node);
    return(TCL_OK);
  } else if (argc == 2 && !strcmp(argv[1], "reset")) {
    perfConsult->resetRefinement();
    sprintf(interp->result, "1");
    return(TCL_OK);
  } else if (argc == 3 && 
	     !strcmp(argv[1], "set") && 
	     (node = atoi(argv[2]) > 0)) {
    sprintf(interp->result, "%d", perfConsult->setCurrentSHGnode(node));
    return TCL_OK;
  } else if (argc == 2 && !strcmp(argv[1], "start")) {
    perfConsult->clearSHG();
    return TCL_OK;
  } else {
    printf("Usage: paradyn shg set <int>\n");
    printf("       paradyn shg get\n");
    printf("       paradyn shg reset\n");
    printf("       paradyn shg start\n");
    return TCL_ERROR;
  }
}

int ParadynSuppressCmd (ClientData clientData,
		       Tcl_Interp *interp,
		       int argc,
		       char *argv[])
{
  bool suppressInst, suppressChildren;

  if (argc != 3) {
    printf("Usage: paradyn suppress <search|inst|searchChildren> <resource list>\n");
    return TCL_ERROR;
  }
  if (!strcmp(argv[1], "search")) {
    suppressInst = false;
    suppressChildren = false;
  } else if (!strcmp(argv[1], "inst")) {
    suppressInst = true;
  } else if (!strcmp(argv[1], "searchChildren")) {
    suppressInst = false;
    suppressChildren = true;
  } else {
    printf("Usage: paradyn suppress <search|inst|searchChildren> <resource list>\n");
    return TCL_ERROR;
  }

  {
    char **argsv;
    int argsc;
    resourceHandle *res;
    
    if (Tcl_SplitList(interp, argv[2], &argsc, &argsv) != TCL_OK) {
      printf("Error parsing resource list '%s'", argv[2]);
      return TCL_ERROR;
    }
    
    cout << "suppress request for ";
    for (int i = 0; i < argsc; i++) {
      res = dataMgr->findResource (argsv[i]);
      cout << argsv[i];
      if (suppressInst) {
	dataMgr->setResourceInstSuppress(*res, true);
      } else {
	if (suppressChildren)
	  dataMgr->setResourceSearchChildrenSuppress(*res, true);
	else
	  dataMgr->setResourceSearchSuppress(*res, true);
      }
      delete res;
    }
    cout << endl;
    free(argsv);
    return TCL_OK;
  }
}

int ParadynVisiCmd (ClientData clientData,
		    Tcl_Interp *interp,
		    int argc,
		    char *argv[])
{
//
//  @begin(barf)
//    This should be automated with a visi command 
//    dictionary or at least a switch statement.  --rbi
//  @end(barf)
//
  if (argc < 2) {
    sprintf(interp->result,
	    "USAGE: visi [kill <ivalue>|create <ivalue>|info|active<cmd>]");
    return TCL_ERROR;
  }
  if (argv[1][0] == 'a') {
    vector<VM_activeVisiInfo> *temp;

    temp = vmMgr->VMActiveVisis();
    for(int i=0; i < temp->size(); i++){
      printf("active_info %d: name %s TypeId %d visiNum = %d\n",i,
	     ((*temp)[i]).name.string_of(),
	     ((*temp)[i]).visiTypeId,((*temp)[i]).visiNum);
    }
    delete temp;
  }
  else if (argv[1][0] == 'i') {
      vector<VM_visiInfo> *visi_info;

      visi_info = vmMgr->VMAvailableVisis();
      for(int i=0; i < visi_info->size();i++){
	printf("visi %d: name %s visiTypeId %d\n",i,
	       ((*visi_info)[i]).name.string_of(), 
	       ((*visi_info)[i]).visiTypeId);
      }
      delete visi_info;
    } 
  else if (argv[1][0] == 'c') {
    int ok, i;
    if (Tcl_GetInt (interp, argv[2], &i) != TCL_OK) 
      return TCL_ERROR;
    ok = vmMgr->VMCreateVisi(1,-1,i,NULL); 
  } 
  else if (argv[1][0] == 'k') {
    int i;
    if (Tcl_GetInt (interp, argv[2], &i) != TCL_OK) 
      return TCL_ERROR;
    vmMgr->VMDestroyVisi(i);
  } 
  else {
    sprintf(interp->result,
	    "USAGE: visi [kill <ivalue>|create <ivalue>|info|active<cmd>]");
    return TCL_ERROR;
  }
  return TCL_OK;
}

static struct cmdTabEntry Pd_Cmds[] = {
  {"pause", ParadynPauseCmd},
  {"cont", ParadynContCmd},
  {"status", ParadynStatusCmd},
  {"list", ParadynListCmd},
  {"daemons", ParadynDaemonsCmd},
  {"detach", ParadynDetachCmd},
  {"disable", ParadynDisableCmd},
  {"enable", ParadynEnableCmd},
  {"gettotal", ParadynGetTotalCmd},
  {"metrics", ParadynMetricsCmd},
  {"print", ParadynPrintCmd},
  {"process", ParadynProcessCmd},
  {"resources", ParadynResourcesCmd},
  {"set", ParadynSetCmd},
  {"core", ParadynCoreCmd},
  {"search", ParadynSearchCmd},
  {"shg", ParadynSHGCmd},
  {"suppress", ParadynSuppressCmd},
  {"visi", ParadynVisiCmd},
  {NULL, NULL}
};

int ParadynCmd(ClientData clientData, 
		Tcl_Interp *interp, 
		int argc, 
		char *argv[])
{
  int i;

  if (argc < 2) {
    sprintf(interp->result,"USAGE: %s <cmd>", argv[0]);
    return TCL_ERROR;
  }

  for (i = 0; Pd_Cmds[i].cmdname; i++) {
    if (strcmp(Pd_Cmds[i].cmdname,argv[1]) == 0) {
      return ((Pd_Cmds[i].func)(clientData,interp,argc-1,argv+1));      
      }
  }

  sprintf(interp->result,"unknown paradyn cmd '%s'",argv[1]);
  return TCL_ERROR;  
}
