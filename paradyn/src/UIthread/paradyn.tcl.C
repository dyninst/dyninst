/* paradyn.tcl.C

   This code implements the tcl "paradyn" command.  See the README file for 
   command descriptions.

*/
/* $Log: paradyn.tcl.C,v $
/* Revision 1.8  1994/04/27 22:55:09  hollings
/* Merged refine auto and search.
/*
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
#include "util/h/tunableConst.h"
#include "VM.CLNT.h"
#include "thread/h/thread.h"
#include "paradyn.h"
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
  resource *parent, *child;
  resourceList *resList, *children;
  int i, j, count, count2;
  char *name;

  parent = uim_rootRes;
  resList = dataMgr->getResourceChildren(parent);

  count = dataMgr->getResourceCount(resList);

  for (i = 0; i < count; i++) {
    parent = dataMgr->getNthResource(resList, i);

    name = dataMgr->getResourceName(parent);
    Tcl_AppendElement(interp, name);

    children = dataMgr->getResourceChildren(parent);
    count2 = dataMgr->getResourceCount(children);

    for (j = 0; j < count2; j++) {
      child = dataMgr->getNthResource(children, j);
      name = dataMgr->getResourceName(child);
      Tcl_AppendElement(interp, name);
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
    printf("%s = %f\n", c->getName(), c->getValue());
  }
  printf("bucketWidth %f\n", dataMgr->getCurrentBucketWidth());
  printf("number of buckets = %d\n", dataMgr->getMaxBins());
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
	       dataMgr->getMetricName(dataMgr->getMetric(mi)), val);
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

    dataMgr->addExecutable(context, machine, user, paradynd, argc-i, &argv[i]);

    return(0);
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
  ret = dataMgr->createResourceList();

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
      child = dataMgr->findChildResource (parent, argv2[el]);
      if (!child) {
	printf ("Resource %s (child of %s) not defined\n", argv2[el], 
		((el == 0) ? "/" : argv2[el-1]) );
	return NULL;
      }
      parent = child;
    }
    dataMgr->addResourceList(ret, child);
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
  char *name;
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
  name = dataMgr->getResourceListName (resList);
  printf ("enable request for %s\n", name);
  delete(name);

  // Now check the metric 
  met = dataMgr->findMetric (context, argv[1]);
  if (!met) {
    sprintf (interp->result, "metric %s is not defined\n", argv[1]);
    return TCL_ERROR;
  }
  else {
    // Finally enable the data collection
    mi = dataMgr->enableDataCollection (uim_defaultStream, resList , met);
    if (mi) {
      uim_enabled.add(mi, (void *) uim_eid);
      sprintf(interp->result,"%d",uim_eid);
      printf ("metric %s, id = %d\n", dataMgr->getMetricName(met), uim_eid);
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
  dataMgr->coreProcess(context, -1);
  return TCL_OK;
}

int ParadynSetCmd (ClientData clientData,
		    Tcl_Interp *interp,
		    int argc,
		    char *argv[])
{
  char *sp;
  tunableConstant *curr;
  float f;
  double d;

  if (argc != 3) {
    sprintf(interp->result,"USAGE: %s <variable> <value>", argv[0]);
    return TCL_ERROR;
  }
  assert (tunableConstant::allConstants && tunableConstant::pool);
  sp = tunableConstant::pool->findAndAdd(argv[1]);
  curr = tunableConstant::allConstants->find(sp);

  if (Tcl_GetDouble(interp, argv[2], &d) == TCL_ERROR)
    return TCL_ERROR;
  else 
    f = (float) d;
  if (curr) {
    if (!curr->setValue(f)) {
      sprintf (interp->result, "value %f not valid.\n", f);
      return TCL_ERROR;
    }
    else {
      printf ("%s set to %f\n", curr->getName(), curr->getValue());
    }
  } else {
    sprintf (interp->result, "variable %s not defined\n", argv[1]);
    return TCL_ERROR;
  }
  return TCL_OK;
}

int ParadynSearchCmd (ClientData clientData,
		      Tcl_Interp *interp,
		      int argc,
		      char *argv[])
{
  int limit;

  if (argc == 3) {
    if (Tcl_GetInt (interp, argv[2], &limit) == TCL_ERROR) 
      return TCL_ERROR;
  } else if (argc == 2) {
    limit = -1;
  } else {
    printf("Usage: paradynd search <false|true> <int>\n");
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
    ok = vmMgr->VMCreateVisi(i); 
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
struct cmdTabEntry 
{
  char *cmdname;
  int (*func)(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);
};

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
