/* paradyn.tcl.C

   This code implements the tcl "paradyn" command.  See the README file for 
   command descriptions.

*/
/* $Log: paradyn.tcl.C,v $
/* Revision 1.27  1994/09/22 01:17:53  markc
/* Cast stringHandles to char*s in printf statements
/*
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

#include <string.h>
extern "C" {
 #include "tcl.h"
}

#include "../DMthread/DMresource.h"
#include "util/h/tunableConst.h"
#include "VM.CLNT.h"
#include "thread/h/thread.h"
#include "../pdMain/paradyn.h"
#include "UIglobals.h"
#include <assert.h>

extern Boolean detachApplication(applicationContext,Boolean);

int ParadynPauseCmd(ClientData clientData, 
		Tcl_Interp *interp, 
		int argc, 
		char *argv[])
{
  dataMgr->pauseApplication(context);
  return TCL_OK;
}


int ParadynContCmd(ClientData clientData, 
		Tcl_Interp *interp, 
		int argc, 
		char *argv[])
{
  dataMgr->continueApplication(context);
  return TCL_OK;
}

int ParadynStatusCmd(ClientData clientData, 
		Tcl_Interp *interp, 
		int argc, 
		char *argv[])
{
  dataMgr->printStatus(context);
  return TCL_OK;
}

int ParadynMetricsCmd(ClientData clientData, 
			Tcl_Interp *interp, 
			int argc, 
			char *argv[])
{
  String_Array ml;
  int i;
  
  ml = dataMgr->getAvailableMetrics(context);
  for (i=0; i < ml.count; i++)
    Tcl_AppendElement(interp, ml.data[i]);
  return TCL_OK;
}

int ParadynResourcesCmd(ClientData clientData, 
			Tcl_Interp *interp, 
			int argc, 
			char *argv[])
{
  stringHandle name;
  resource *parent, *child;
  resourceList *resList, *children;
  int i, j, count, count2;

  parent = uim_rootRes;
  resList = parent->getChildren();

  count = resList->getCount();

  for (i = 0; i < count; i++) {
    parent = resList->getNth(i);

    name = parent->getFullName();
    Tcl_AppendElement(interp, (char *) name);

    children = parent->getChildren();
    count2 = children->getCount();

    for (j = 0; j < count2; j++) {
      child = children->getNth(j);
      name = child->getFullName();
      Tcl_AppendElement(interp, (char *) name);
    }
  }
  return TCL_OK;
}

int ParadynListCmd(ClientData clientData, 
		Tcl_Interp *interp, 
		int argc, 
		char *argv[])
{
  String_Array ml;
  tunableConstant *c;
  int i;
  List<tunableConstant*> curr;

  dataMgr->printResources();
  ml = dataMgr->getAvailableMetrics(context);
  for (i=0; i < ml.count; i++) {
    printf("%s\n", ml.data[i]);
  }
  printf("CONSTANTS\n");
  assert(tunableConstant::allConstants);
  for (curr = *tunableConstant::allConstants; c = *curr; curr++) {
    c->print();
  }
  printf("bucketWidth %f\n", dataMgr->getCurrentBucketWidth());
  printf("number of buckets = %d\n", dataMgr->getMaxBins());
  dataMgr->printDaemons(context);
  return TCL_OK;
}

int ParadynDetachCmd (ClientData clientData,
		      Tcl_Interp *interp,
		      int argc,
		      char *argv[])
{
  dataMgr->detachApplication(context, True);
  return TCL_OK;
}

int ParadynGetTotalCmd (ClientData clientData,
		     Tcl_Interp *interp,
		     int argc,
		     char *argv[])
{
  int met;
  metricInstance *mi;
  float val;

  if (argc < 2) {
    sprintf(interp->result, "USAGE: gettotal <metid>");
    return TCL_ERROR;
  }

  if (Tcl_GetInt(interp, argv[1], &met) == TCL_ERROR) {
    sprintf(interp->result, "Could not parse '%s' as an integer", argv[1]);
    return TCL_ERROR;
  }

  mi = uim_enabled.find((void *) met);

  if (!mi) {
    sprintf (interp->result, "unable to find metric %d\n", met); 
    return TCL_ERROR;
  }
  else {
    val = dataMgr->getTotValue(mi);
    sprintf(interp->result, "%g", val);
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
    metricInstance *mi;
    int met;

    if (Tcl_GetInt(interp, argv[2], &met) == TCL_ERROR) 
      return TCL_ERROR;
    mi = uim_enabled.find((void *) met);
    if (!mi) {
      sprintf (interp->result, "unable to find metric %d\n", met); 
      return TCL_ERROR;
     } else {
      val = dataMgr->getMetricValue(mi);
      printf ("metric %s, val = %f\n", 
	       (char*)dataMgr->getMetricName(dataMgr->getMetric(mi)), val);
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

    if (dataMgr->addExecutable(context, machine, user, paradynd, (char*)0,
			       argc-i, &argv[i]) == False)
      return TCL_ERROR;
    else
      return TCL_OK;
  }

/*
 * build_resource_list
 * parses string of form <aaa/bbb/ccc,ddd/eee>, building up a 
 * list of resources which is returned.
 * returns NULL if any resource not defined or if argument does not 
 * match regular expression for the string.
 */
resourceList *build_resource_list (Tcl_Interp *interp, char *list)
{
  char **argv1, **argv2;
  int argc1, argc2;
  resourceList *ret;
  resource *parent, *child;
  int res, el;

  printf("list is %s\n",list);
  ret = new resourceList;

  if (Tcl_SplitList(interp, list, &argc1, &argv1) != TCL_OK) {
    printf("Could not split list '%s'", list);
    return NULL;
  }

  for (res = 0; res < argc1; res++) {
    if (Tcl_SplitList(interp, argv1[res], &argc2, &argv2) != TCL_OK) {
      printf("Could not split list '%s'", argv1[res]);
      return NULL;
    }
    parent = uim_rootRes;
    for (el = 0; el < argc2; el++) {
      child = parent->findChild(argv2[el]);
      if (!child) {
	printf ("Resource %s (child of %s) not defined\n", argv2[el], 
		((el == 0) ? "/" : argv2[el-1]) );
	return NULL;
      }
      parent = child;
    }
    ret->add(child);
    free(argv2);
  }

  free(argv1);
  return ret;
}

//
//  disable  <metid>
//
int ParadynDisableCmd (ClientData clientData,
		      Tcl_Interp *interp,
		      int argc,
		      char *argv[])
{
  int met;
  metricInstance *mi;

  // Hold Everything!
  dataMgr->pauseApplication (context);

  if (argc < 2) {
    sprintf(interp->result, "USAGE: disable <metid>");
    return TCL_ERROR;
  }

  if (Tcl_GetInt(interp, argv[1], &met) == TCL_ERROR) {
    sprintf(interp->result, "Could not parse '%s' as an integer", argv[1]);
    return TCL_ERROR;
  }

  mi = uim_enabled.find((void *) met);

  if (!mi) {
    sprintf (interp->result, "unable to find metric %d\n", met); 
    return TCL_ERROR;
  }
  else {
    dataMgr->disableDataCollection (uim_defaultStream, mi);
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
  metric *met;
  stringHandle name;
  metricInstance *mi;
  resourceList *resList;

  // Hold Everything!
  dataMgr->pauseApplication (context);

  // Build a resource list from the tcl list
  if (argc == 2)
    resList = dataMgr->getRootResources();
  else {
    resList = build_resource_list (interp, argv[2]);
    if (resList == NULL) {
      sprintf (interp->result, "unable to build resource list for %s",
	       argv[2]);
      return TCL_ERROR;
    }
  }

  // DEBUG
  name = resList->getCanonicalName();
  printf ("enable request for %s\n", (char*) name);

  // Now check the metric 
  met = dataMgr->findMetric (context, argv[1]);
  if (!met) {
    sprintf (interp->result, "metric %s is not defined\n", argv[1]);
    return TCL_ERROR;
  }
  else {
    // Finally enable the data collection
    mi = dataMgr->enableDataCollection (uim_defaultStream, resList,
					met);
    if (mi) {
      uim_enabled.add(mi, (void *) uim_eid);
      sprintf(interp->result,"%d",uim_eid);
      printf ("metric %s, id = %d\n", (char*) dataMgr->getMetricName(met), uim_eid);
      uim_eid++;
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
  pid = atoi(argv[1]);
  dataMgr->coreProcess(context, pid);
  return TCL_OK;
}

int ParadynSetCmd (ClientData clientData,
		    Tcl_Interp *interp,
		    int argc,
		    char *argv[])
{
  int val;
  stringHandle sp;
  tunableConstant *curr;
  tunableFloatConstant *fConst;
  tunableBooleanConstant *bConst;
  float f;
  double d;

  if (argc != 3) {
    sprintf(interp->result,"USAGE: %s <variable> <value>", argv[0]);
    return TCL_ERROR;
  }
  assert (tunableConstant::allConstants && tunableConstant::pool);
  sp = tunableConstant::pool->findAndAdd(argv[1]);
  curr = tunableConstant::allConstants->find(sp);

  if (!curr) {
    sprintf (interp->result, "variable %s not defined\n", argv[1]);
    return TCL_ERROR;
  } else if (curr->getType() == tunableFloat) {
      fConst = (tunableFloatConstant *) curr;
      if (Tcl_GetDouble(interp, argv[2], &d) == TCL_ERROR) {
	return TCL_ERROR;
      } else {
	f = (float) d;
      }

      if (!fConst->setValue(f)) {
	  sprintf (interp->result, "value %f not valid.\n", f);
	  return TCL_ERROR;
      } else {
	  printf ("%s set to %f\n", (char*) fConst->getName(), fConst->getValue());
      }
  } else if (curr->getType() == tunableBoolean) {
      bConst = (tunableBooleanConstant *) curr;
      if (Tcl_GetBoolean(interp, argv[2], &val) == TCL_ERROR) {
	  return TCL_ERROR;
      }

      if (val) {
	  bConst->setValue(True);
	  printf ("%s set to True\n", (char*)bConst->getName());
      } else {
	  bConst->setValue(False);
	  printf ("%s set to False\n", (char*)bConst->getName());
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

  if (dataMgr->applicationDefined(context) != True) {
    sprintf (interp->result, "no program defined, can't search\n");
    return TCL_ERROR;
  } else {
    perfConsult->search(True, limit);
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
    int i;
    int limit;
    resource *r;
    stringHandle name;
    Boolean suppressInst;
    resourceList *resList;

    if (argc != 3) {
	printf("Usage: paradyn suppress <search|inst> <resource list>\n");
	return TCL_ERROR;
    }
    resList = build_resource_list (interp, argv[2]);
    if (resList == NULL) {
      sprintf (interp->result, "unable to build resource list for %s",
	       argv[2]);
      return TCL_ERROR;
    }

    if (!strcmp(argv[1], "search")) {
	suppressInst = False;
    } else if (!strcmp(argv[1], "inst")) {
	suppressInst = True;
    } else {
	sprintf (interp->result, "suppress option (%s) not search or inst",
	    argv[1]);
      return TCL_ERROR;
    }

    for (i = 0, limit = resList->getCount(); i < limit; i++) {
	// DEBUG
	r = resList->getNth(i);
	name = r->getName();
	printf ("suppress request for %s\n", (char*)name);

	if (suppressInst) {
	    dataMgr->setResourceInstSuppress(context, r, TRUE);
	} else {
	    dataMgr->setResourceSearchSuppress(context, r, TRUE);
	}
    }
    return TCL_OK;
}

int ParadynVisiCmd (ClientData clientData,
		    Tcl_Interp *interp,
		    int argc,
		    char *argv[])
{
  if (argv[1][0] == 'a') {
    VM_activeVisiInfo_Array temp;
    int i;

    temp = vmMgr->VMActiveVisis();
    for(i=0;i<temp.count;i++){
      printf("active_info %d: name %s TypeId %d visiNum = %d\n",i,
	     temp.data[i].name,temp.data[i].visiTypeId,temp.data[i].visiNum);
    }
  }
  else if (argv[1][0] == 'i') {
      VM_visiInfo_Array visi_info;
      int i;

      visi_info = vmMgr->VMAvailableVisis();
      for(i=0;i<visi_info.count;i++){
	printf("visi %d: name %s visiTypeId %d\n",i,
	       visi_info.data[i].name,visi_info.data[i].visiTypeId);
      }
    } 
  else if (argv[1][0] == 'c') {
    int ok, i;
    if (Tcl_GetInt (interp, argv[2], &i) != TCL_OK) 
      return TCL_ERROR;
    ok = vmMgr->VMCreateVisi(1,i,NULL,NULL); 
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

void initParadynCmd(Tcl_Interp *interp)
{
  Tcl_CreateCommand(interp, "paradyn", ParadynCmd, (ClientData) NULL,
		    (Tcl_CmdDeleteProc *) NULL);

  return;
}
