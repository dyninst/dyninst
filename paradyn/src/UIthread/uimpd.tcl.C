/* uimpd.C
   this file contains implementation of "uimpd" tcl command.  This command
   is used internally by the UIM.
*/
/* $Log: uimpd.tcl.C,v $
/* Revision 1.10  1994/10/25 17:57:37  karavan
/* added Resource Display Objects, which support display of multiple resource
/* abstractions.
/*
 * Revision 1.9  1994/10/09  02:28:07  karavan
 * Many updates related to the switch to the new UIM/visiThread resource/metric
 * selection interface, and to resource selection directly on the nodes.
 *
 * Revision 1.8  1994/09/25  01:54:14  newhall
 * updated to support changes in VM, and UI interface
 *
 * Revision 1.7  1994/09/24  01:10:00  rbi
 * Added #include of stdlib.h to get correct prototype for atof()
 * and thereby fix the SHG display bug.
 *
 * Revision 1.6  1994/09/22  01:17:26  markc
 * Added const to char* for args in compare function
 *
 * Revision 1.5  1994/09/21  15:35:24  karavan
 * added addNStyle and addEStyle commands
 *
 * Revision 1.4  1994/08/01  20:24:42  karavan
 * new version of dag; new dag support commands
 *
 * Revision 1.3  1994/05/12  23:34:18  hollings
 * made path to paradyn.h relative.
 *
 * Revision 1.2  1994/05/07  23:26:48  karavan
 * added short explanation feature to shg.
 *
 * Revision 1.1  1994/05/05  19:54:03  karavan
 * initial version.
 * */
 
#include <stdlib.h>

extern "C" {
  #include "tk.h"
  int atoi(const char*);
}
#include "../pdMain/paradyn.h"
#include "../DMthread/DMresource.h"
#include "UIglobals.h"
#include "../VMthread/metrespair.h"
#include "dag.h"

extern resourceList *build_resource_list (Tcl_Interp *interp, char *list);
extern int getDagToken ();
extern int initWhereAxis (dag *wheredag, stringHandle abs); 
resourceList *uim_SelectedFocus;

void printMFPlist (metrespair *list, int lsize) 
{
  for (int i = 0; i < lsize; i++) {
    printf ("   metric: %s ||| focus: %s\n", 
	    (char *) dataMgr->getMetricName (list[i].met),
	    (char *) (list[i].focus)->getCanonicalName());
  }
}

/* 
   Sends metric-focus pair representation of menu selections to requesting
   visi thread.  Pairs are collected in global list uim_VisiSelections, 
   number of pairs is in uim_VisiSelectionsSize.
   arguments:
       0: sendVisiSelections
       1: msgID
       2: cancelFlag

   note: space allocated for chosenMets and localFocusList must be 
         freed by the visi thread which gets the callback.
*/
int sendVisiSelectionsCmd(ClientData clientData, 
		Tcl_Interp *interp, 
		int argc, 
		char *argv[])
{
  Tcl_HashEntry *entry;
  UIMReplyRec *msgRec;
  chooseMandRCBFunc mcb;
  int msgID, cancelFlag;
  List<metrespair *> localSelectList;
  metrespair *metricFocusPairs = NULL;


  printf ("sendVisiSelectionsCmd: %s %s\n", argv[1], argv[2]);

  cancelFlag = atoi(argv[2]);
  if (cancelFlag == 1) {
    uim_VisiSelectionsSize = 0;
    uim_VisiSelections.removeAll();
  }    
  if (uim_VisiSelectionsSize > 0) {
    // build array of metrespair's
    if ((metricFocusPairs = new metrespair [uim_VisiSelectionsSize]) == NULL) {
      printf ("malloc error!\n");
      thr_exit(0);
    }
    localSelectList = uim_VisiSelections;
    for (int i = 0; i < uim_VisiSelectionsSize; i++) {
      metricFocusPairs[i].met = (*localSelectList)->met;
      metricFocusPairs[i].focus = (*localSelectList)->focus;
      localSelectList++;
    }
    printMFPlist (metricFocusPairs, uim_VisiSelectionsSize);
  }

  // get callback and thread id for this msg
  msgID = atoi(argv[1]);
  if (!(entry = Tcl_FindHashEntry (&UIMMsgReplyTbl, (char *) msgID))) {
    Tcl_AppendResult (interp, "invalid message ID!", (char *) NULL);
    return TCL_ERROR;
  }
  msgRec = (UIMReplyRec *) Tcl_GetHashValue(entry);

     /* set thread id for return */
  uim_server->setTid(msgRec->tid);
  mcb = (chooseMandRCBFunc) msgRec->cb;

  uim_server->chosenMetricsandResources(mcb, metricFocusPairs, 
					uim_VisiSelectionsSize);
  // cleanup data structures
  Tcl_DeleteHashEntry (entry);   // cleanup hash table record
  uim_VisiSelectionsSize = 0;
//  uim_VisiSelections.removeAll(); *** no way to clear list w/out delete!
  return TCL_OK;
}

/* arguments:
       0: processVisiSelection
       1: list of selected metrics
*/
int processVisiSelectionCmd(ClientData clientData, 
			    Tcl_Interp *interp, 
			    int argc, 
			    char *argv[])
{
  int metcnt, metindx;
  metrespair *mfp;
  char **metlst;

  if (Tcl_SplitList (interp, argv[1], &metcnt, &metlst) == TCL_OK) {
    for (int i = 0; i < metcnt; i++) {
      metindx = atoi(metlst[i]);
      mfp = new metrespair;
      mfp->met = dataMgr->findMetric(context, uim_AvailMets.data[metindx]);
      mfp->focus = uim_SelectedFocus;
      uim_VisiSelections.add (mfp);
      uim_VisiSelectionsSize++;
    }
    free (metlst);
  }
  return TCL_OK;
}

/*
  closeDAGCmd
  binding set in tcl procedure initWHERE,
  looks up dag * for this display and calls function to close display
  arguments: dagID
*/
int closeDAGCmd (ClientData clientData, 
		   Tcl_Interp *interp, 
		   int argc, 
		   char *argv[])
{
  int dagID;
  dag *currDag;
  dagID = atoi(argv[1]);
  currDag = ActiveDags[dagID];
  currDag->destroyDisplay();
  printf ("dag %d destroyed\n", dagID);
  return TCL_OK;
}

/*
  addEStyleCmd
  looks up dag * for this display and calls function to add Edge Style
  arguments: dagID, styleID, arrow, fill, capstyle, width
*/
int addEStyleCmd (ClientData clientData, 
		   Tcl_Interp *interp, 
		   int argc, 
		   char *argv[])
{
  int dagID;
  dag *currDag;
  dagID = atoi(argv[1]);
  currDag = ActiveDags[dagID];

  currDag->AddEStyle(atoi(argv[2]), atoi(argv[3]),  0, 0, 0, NULL, 
		     argv[4], argv[5][0], atof(argv[6]));
  return TCL_OK;
}

/*
  addNStyleCmd
  looks up dag * for this display and calls function to add node Style
  arguments: dagID, styleID, bg, outline, font, text, shape, 
             width
*/
int addNStyleCmd (ClientData clientData, 
		   Tcl_Interp *interp, 
		   int argc, 
		   char *argv[])
{
  int dagID;
  dag *currDag;
  dagID = atoi(argv[1]);
  printf ("adding style for dagtoken = %d\n", dagID);
  currDag = ActiveDags[dagID];
  currDag->AddNStyle(atoi(argv[2]), argv[3], argv[4], NULL,
		     argv[5], argv[6], argv[7][0], atof(argv[8]));
  return TCL_OK;
}

/*
  refineSHGCmd
  binding set in tcl procedure initWHERE,
  looks up dag * for this display and calls function to close display
  arguments: global variable currentSelection$dagID
*/
int refineSHGCmd (ClientData clientData, 
		  Tcl_Interp *interp, 
		  int argc, 
		  char *argv[])
{
  int nodeID;
  nodeID = atoi(argv[1]);
  if (nodeID < 0) {
    sprintf (interp->result, "no selection currently defined\n");
    return TCL_ERROR;
  }
  perfConsult->setCurrentSHGnode (nodeID);
  if (dataMgr->applicationDefined(context) != True) {
    sprintf (interp->result, "no program defined, can't search\n");
    return TCL_ERROR;
  } else {
    perfConsult->search(True, 1);
    return TCL_OK;
  }
}


/*
  hideSubgraphCmd
  binding set in tcl procedure initWHERE,
  looks up dag * for this display and calls function to close display
  arguments: dagID
             currentselection variable name
*/
int hideSubgraphCmd (ClientData clientData, 
		     Tcl_Interp *interp, 
		     int argc, 
		     char *argv[])
{
  int nodeID, dagID;
  char *currNode;
  dag *currDag;
  currNode = Tcl_GetVar (interp, argv[2], TCL_GLOBAL_ONLY);
  if (currNode == NULL)
    return TCL_ERROR;
  nodeID = atoi(currNode);
  if (nodeID < 0) {
    sprintf (interp->result, "no selection currently defined\n");
    return TCL_ERROR;
  }
  dagID = atoi(argv[1]);
  currDag = ActiveDags[dagID];
  currDag->addDisplayOption (SUBTRACT, nodeID);
  return TCL_OK;
}

/*
  showAllNodesCmd
  binding set in tcl procedure initWHERE,
  looks up dag * for this display and calls function to close display
  arguments: dagID
             currentselection variable name
*/
int showAllNodesCmd (ClientData clientData, 
		     Tcl_Interp *interp, 
		     int argc, 
		     char *argv[])
{
  int dagID;
  dag *currDag;
  dagID = atoi(argv[1]);
  currDag = ActiveDags[dagID];
  currDag->clearAllDisplayOptions ();
  return TCL_OK;
}

/*
  showWhereAxisCmd
  binding set in tcl procedure initWHERE,
  looks up dag * for this display and calls function to display
  arguments: dagID
*/
int showWhereAxisCmd (ClientData clientData, 
		      Tcl_Interp *interp, 
		      int argc, 
		      char *argv[])
{
  /*** this for compile only --- arguments to initWhereAxis have changed!!!
  if (initWhereAxis (baseWhere, 0))
*/
    return TCL_OK;
/***  else 
    return TCL_ERROR;
*/
}

/*
  shgShortExplain
  called from tcl procedure shgFullName.
  looks up and displays full pathname for node.
  arguments: 0 - cmd name
             1 - nodeID
*/  
int shgShortExplainCmd (ClientData clientData, 
                Tcl_Interp *interp, 
                int argc, 
                char *argv[])
{
  Tcl_HashEntry *entry;
  Tk_Uid nodeID;
  char *nodeExplain;

  printf ("shgShortExplain\n");
  // get string for this nodeID
  nodeID = Tk_GetUid (argv[1]);
  if (!(entry = Tcl_FindHashEntry (&shgNamesTbl, nodeID))) {
    Tcl_AppendResult (interp, "invalid message ID!", (char *) NULL);
    return TCL_ERROR;
  }
  nodeExplain = (char *) Tcl_GetHashValue(entry);
    // change variable linked to display window; display window will be 
    //  updated automatically
  Tcl_SetVar (interp, "shgExplainStr", nodeExplain, TCL_GLOBAL_ONLY);
  return TCL_OK;
}

/*
  highlightNodeCmd
  called from tcl procedure updateCurrentSelection.
  looks up and displays full pathname for node.
  arguments: 0 - cmd name
             1 - nodeID
	     2 - dag id
	     3 - selFlag (opt)
*/  
int highlightNodeCmd (ClientData clientData, 
		      Tcl_Interp *interp, 
		      int argc, 
		      char *argv[])
{
  int nodeID, dagID;
  dag *currDag;

  // get string for this nodeID
  nodeID = atoi(argv[1]);
  dagID = atoi (argv[2]);

  currDag = ActiveDags[dagID];
  if (argc > 3) {
    if (currDag->constrHighlightNode (nodeID)) 
      return TCL_OK;    
    else
      return TCL_ERROR;
  }
    
  if (currDag->highlightNode (nodeID)) 
    return TCL_OK;    
  else {
    return TCL_ERROR;
  }
}

/*
  unhighlightNodeCmd
  called from tcl procedure updateCurrentSelection.
  looks up and displays full pathname for node.
  arguments: 0 - cmd name
             1 - nodeID
	     2 - dag id
*/  
int unhighlightNodeCmd (ClientData clientData, 
		      Tcl_Interp *interp, 
		      int argc, 
		      char *argv[])
{
  int nodeID, dagID;
  dag *currDag;

  // get string for this nodeID
  nodeID = atoi(argv[1]);
  dagID = atoi (argv[2]);

  currDag = ActiveDags[dagID];
  if (currDag->unhighlightNode (nodeID)) 
    return TCL_OK;    
  else {
    return TCL_ERROR;
  }
}

/* 
 * args: 0 processResourceSelection
 *       1 rdo token
 */
int processResourceSelectionCmd (ClientData clientData, 
                      Tcl_Interp *interp, 
                      int argc, 
                      char *argv[])
{
  int rdoToken;
  dag *currDag;
  resourceList *selection;
  rNode me;
  int r;
  resourceDisplayObj *currRDO;

  // get rdo ptr from token
  rdoToken = atoi(argv[1]);
  printf ("processResourceSelection::rdoToken: %d\n", rdoToken);
  currRDO = resourceDisplayObj::allRDOs.find((void *)rdoToken);
  printf ("processResourceSelection::lookuptoken: %d\n", currRDO->getToken());

  // get currently displayed dag
  currDag = currRDO->getTopDag();
  printf ("processResourceSelection::lookupDag: %s\n", currDag->getCanvasName());
  selection = new resourceList;
  for (r = 0; r < currDag->graph->rSize; r++) {
    me = currDag->graph->row[r].first;
    while (me != NULL) {
      if (currDag->isHighlighted (me))
        selection->add ((resource *) me->aObject);
      me = me->forw;
    }
  }
  printf ("DONE WITH LOOP\n");
  if (selection->getCount() == 0)
    return TCL_ERROR;
  uim_SelectedFocus = selection;
#ifdef UIM_DEBUG
  printf ("resource selection %s added\n", 	    
	  (char *) uim_SelectedFocus->getCanonicalName());
#endif
  return TCL_OK;
}

int clearResourceSelectionCmd (ClientData clientData, 
                      Tcl_Interp *interp, 
                      int argc, 
                      char *argv[])
{
  int dagID;
  dag *currDag;

  // get dag ptr from token
  dagID = atoi (argv[1]);

  currDag = ActiveDags[dagID];
  currDag->clearAllHighlighting();
  currDag->highlightAllRootNodes();
  return TCL_OK;
}
  
/* 
   drawStartVisiMenuCmd
   gets list of currently available visualizations from visi manager
   and displays menu which allows 0 or more selections.
   Tcl command "drawVisiMenu" will return selections to the requesting
   visi thread
*/
int compare_visi_names (void *viptr1, void *viptr2) {
  const VM_visiInfo *p1 = (VM_visiInfo *)viptr1;
  const VM_visiInfo *p2 = (VM_visiInfo *)viptr2;
  return strcmp (p1->name, p2->name);
}


/* 
   drawStartVisiMenuCmd
   gets list of currently available visualizations from visi manager
   and displays menu which allows 0 or more selections.
   Tcl command "drawVisiMenu" will return selections to the requesting
   visi thread
*/
int compare_visi_names (const void *viptr1, const void *viptr2) {
  const VM_visiInfo *p1 = (VM_visiInfo *)viptr1;
  const VM_visiInfo *p2 = (VM_visiInfo *)viptr2;
  return strcmp (p1->name, p2->name);
}

int drawStartVisiMenuCmd (ClientData clientData, 
                Tcl_Interp *interp, 
                int argc, 
                char *argv[])
{
  int i;
  VM_visiInfo_Array via;
  int count;
  VM_visiInfo *vinfo;
  Tcl_DString namelist;
  Tcl_DString numlist;
  char num[8];

  via = vmMgr->VMAvailableVisis();
  count = via.count;
  vinfo = via.data;
  
  qsort (vinfo, count, sizeof(VM_visiInfo), compare_visi_names);
  Tcl_DStringInit(&namelist);
  Tcl_DStringInit(&numlist);
  
  for (i = 0; i < count; i++) {
    Tcl_DStringAppendElement(&namelist, vinfo[i].name);
    sprintf (num, "%d", vinfo[i].visiTypeId);
    Tcl_DStringAppendElement(&numlist, num);
  }
  Tcl_SetVar (interp, "vnames", Tcl_DStringValue(&namelist), 0);
  Tcl_SetVar (interp, "vnums", Tcl_DStringValue(&numlist), 0);
  Tcl_DStringFree (&namelist);
  Tcl_DStringFree (&numlist);
  sprintf (num, "%d", count);
  Tcl_SetVar (interp, "vcount", num, 0);
  Tcl_SetVar (interp, "title", argv[1], 0);

  if (Tcl_VarEval (interp, "drawVisiMenu $title $vnames $vnums $vcount",
		   0) == TCL_ERROR) {
    printf ("%s", interp->result);
    return TCL_ERROR;
  }
  return TCL_OK;
}

/*
 * argv[1] = rdoToken
 * argv[2] = abstraction
 */
int switchRDOdagCmd (ClientData clientData, 
                Tcl_Interp *interp, 
                int argc, 
                char *argv[])
{
  int rdoToken;
  resourceDisplayObj *trdo;
  rdoToken = atoi(argv[1]);
  printf ("switchRDOdag %d\n", rdoToken);
  trdo = resourceDisplayObj::allRDOs.find((void *)rdoToken);
  trdo->cycle(argv[2]);
  return TCL_OK;
}



struct cmdTabEntry uimpd_Cmds[] = {
  {"drawStartVisiMenu", drawStartVisiMenuCmd},
  {"sendVisiSelections", sendVisiSelectionsCmd},
  {"shgShortExplain", shgShortExplainCmd},
  {"closeDAG", closeDAGCmd}, 
  {"showWhereAxis", showWhereAxisCmd},
  {"highlightNode", highlightNodeCmd},
  {"unhighlightNode", unhighlightNodeCmd},
  {"refineSHG", refineSHGCmd},
  {"hideSubgraph", hideSubgraphCmd},
  {"showAllNodes", showAllNodesCmd},
  {"addEStyle", addEStyleCmd},
  {"addNStyle", addNStyleCmd},
  {"processVisiSelection", processVisiSelectionCmd},
  {"clearResourceSelection", clearResourceSelectionCmd},
  {"processResourceSelection", processResourceSelectionCmd},
  {"switchRDOdag", switchRDOdagCmd},
 {NULL, NULL}
};

int UimpdCmd(ClientData clientData, 
		Tcl_Interp *interp, 
		int argc, 
		char *argv[])
{
  int i;

  if (argc < 2) {
    sprintf(interp->result,"USAGE: %s <cmd>", argv[0]);
    return TCL_ERROR;
  }

  for (i = 0; uimpd_Cmds[i].cmdname; i++) {
    if (strcmp(uimpd_Cmds[i].cmdname,argv[1]) == 0) {
      return ((uimpd_Cmds[i].func)(clientData,interp,argc-1,argv+1));      
      }
  }

  sprintf(interp->result,"unknown UIM cmd '%s'",argv[1]);
  return TCL_ERROR;  
}

