/* paradyn.tcl.C

   This code implements the tcl "paradyn" command.  See the README file for 
   command descriptions.

*/
/* $Log: paradyn.tcl.C,v $
/* Revision 1.1  1994/04/05 04:42:38  karavan
/* initial version of UI thread code and tcl paradyn command
/* */

#include <string.h>
extern "C" {
 #include "tcl.h"
}
#include "util/h/tunableConst.h"
/*
 #include "visiMgrCLNT.h"
*/
#include "thread/h/thread.h"
#include "paradyn.h"
#include "UIglobals.h"

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

int ParadynListCmd(ClientData clientData, 
		Tcl_Interp *interp, 
		int argc, 
		char *argv[])
{
  dataMgr->printResources();
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

int ParadynPrintCmd (ClientData clientData,
		     Tcl_Interp *interp,
		     int argc,
		     char *argv[])
{
  if (argv[0][1] == 'm') {   // print metric
    float val;
    metricInstance *mi;
    int met;

    if (Tcl_GetInt(interp, argv[1], &met) == TCL_ERROR) 
      return TCL_ERROR;
    mi = uim_enabled.find((void *) met);
    if (!mi) {
      sprintf (interp->result, "unable to find metric %d\n", met); 
      return TCL_ERROR;
     }
    else {
      val = dataMgr->getMetricValue(mi);
      printf ("metric %s, val = %f\n", 
	       dataMgr->getMetricName(dataMgr->getMetric(mi)), val);
    }
  } else
    if (argv[0][1] == 's') {     //print shg
      perfConsult->printSHGList();
    } else
      if (argv[0][1] == 'r') {     // print refine
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
      }
  else {
    sprintf (interp->result, "Unknown option: paradyn print %s\n", 
	     argv[1]);
    return TCL_ERROR;
  }
  return TCL_OK;
}

/*
 * build_resource_list
 * parses string of form <aaa/bbb/ccc,ddd/eee>, building up a 
 * list of resources which is returned.
 * returns NULL if any resource not defined or if argument does not 
 * match regular expression for the string.
 */
resourceList *build_resource_list (Tcl_Interp *interp, char *nameStr)
{
  resourceList *ret;
  resource *parent, *child;
  char *name, *pathname;
  char re[] = "(<)([A-Z]|[a-z]|_|\.|\[|\])*(,[A-Z]|[a-z]|_|\.|\[|\])+(>)";

  if (!Tcl_RegExpMatch(interp, nameStr, re))
    return NULL;

  ret = dataMgr->createResourceList();
  parent = uim_rootRes;
  char comma[] = ",";
  char slash[] = "/";

   // strip off surrounding brackets
  nameStr[strlen(nameStr) - 1] = '\0';
  ++nameStr;
  while ((pathname = strtok(nameStr, comma)) != NULL) {
    while ((name = strtok (pathname, slash)) != NULL) {
      child = dataMgr->findChildResource (parent, name);
      if (!child) {
	printf ("resource %s not defined\n", name);
	return NULL;
      }
      parent = child;
    }
    dataMgr->addResourceList (ret, child);
  }
  return ret;
}

int ParadynEnableCmd (ClientData clientData,
		      Tcl_Interp *interp,
		      int argc,
		      char *argv[])
{
  metric *met;
  char *name;
  metricInstance *mi;
  resourceList *resList;

  dataMgr->pauseApplication (context);

  if (argc == 2)
    resList = dataMgr->getRootResources();
  else {

    // parse and check full resource list
    resList = build_resource_list (interp, argv[2]);
    if (resList == NULL) {
      sprintf (interp->result, "unable to build resource list for %s",
	       argv[2]);
      return TCL_ERROR;
    }
  }
  name = dataMgr->getResourceListName (resList);
  printf ("enable request for %s\n", name);
  delete(name);

  met = dataMgr->findMetric (context, argv[1]);
  if (!met) {
    sprintf (interp->result, "metric %s is not defined\n", argv[1]);
    return TCL_ERROR;
  }
  else {
    mi = dataMgr->enableDataCollection (uim_defaultStream, resList , met);
    if (mi) {
      uim_enabled.add(mi, (void *) uim_eid);
      printf ("metric %s, id = %d\n", dataMgr->getMetricName(met), uim_eid++);
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
  sp = tunableConstant::pool->findAndAdd(argv[0]);
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

int ParadynRefineCmd (ClientData clientData,
		      Tcl_Interp *interp,
		      int argc,
		      char *argv[])
{
  if (argc == 3) {
    int i;
    if (Tcl_GetInt (interp, argv[2], &i) == TCL_ERROR) 
      return TCL_ERROR;
    perfConsult->autoRefine(i);
  }
  else 
    perfConsult->autoRefine(-1);
  return TCL_OK;
}

int ParadynSearchCmd (ClientData clientData,
		      Tcl_Interp *interp,
		      int argc,
		      char *argv[])
{
  if (dataMgr->applicationDefined(context) != True) {
    sprintf (interp->result, "no program defined, can't search\n");
    return TCL_ERROR;
  }
  else {
    perfConsult->search(True);
    return TCL_OK;
  }
}
/*
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
*/
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
  {"enable", ParadynEnableCmd},
  {"print", ParadynPrintCmd},
  {"set", ParadynSetCmd},
  {"core", ParadynCoreCmd},
  {"refine", ParadynRefineCmd},
  {"search", ParadynSearchCmd},
//  {"visi", ParadynVisiCmd},
  {NULL, NULL}
};

int ParadynCmd(ClientData clientData, 
		Tcl_Interp *interp, 
		int argc, 
		char *argv[])
{
  int i;

  printf("In Paradyn Command!\n");

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
